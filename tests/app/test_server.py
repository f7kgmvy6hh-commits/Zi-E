from fastapi.testclient import TestClient

from app.server.config import Settings
from app.server.main import create_app
from app.voice.stt import LocalTranscriber


TOKEN = "test-token-with-at-least-24-characters"


def settings(tmp_path):
    return Settings("127.0.0.1", 8765, TOKEN, True, tmp_path / "log.jsonl", None, "session-one", None, "voice-one", None)


def real_settings(tmp_path):
    cfg = settings(tmp_path)
    return Settings(
        cfg.host, cfg.port, cfg.auth_token, False, cfg.log_path, cfg.hermes_command,
        cfg.hermes_session, cfg.elevenlabs_api_key, cfg.voice_id,
        cfg.voice_fallback_command,
    )


def test_health_is_narrow_and_reports_real_metrics(tmp_path):
    with TestClient(create_app(settings(tmp_path))) as client:
        assert client.get("/api/health").status_code == 401
        response = client.get("/api/health", headers={"Authorization": f"Bearer {TOKEN}"})
        assert response.status_code == 200
        body = response.json()
        assert body["status"] == "ok"
        assert 0 <= body["system"]["cpu_percent"] <= 100
        assert body["system"]["source"] == "local-machine"


def test_stt_rejects_empty_audio_without_loading_model(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        assert client.post("/api/stt/transcribe", headers=headers, content=b"").status_code == 422


def test_state_and_robot_command_publish_websocket_event(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        with client.websocket_connect("/api/events") as websocket:
            websocket.send_json({"authorization": f"Bearer {TOKEN}"})
            response = client.post("/api/robot/command", headers=headers, json={"target": "SAFE", "timeout": 1})
            assert response.status_code == 200
            event = websocket.receive_json()
            assert event["type"] == "robot.command"
            assert event["payload"]["target"] == "SAFE"
            state_event = websocket.receive_json()
            assert state_event["type"] == "robot.state"
            assert state_event["payload"]["source"] == "simulation"
            telemetry_event = websocket.receive_json()
            assert telemetry_event["type"] == "robot.telemetry"
            assert telemetry_event["payload"]["mode"] == "SIMULATION"
        state = client.get("/api/state", headers=headers).json()
        assert state["robot"]["state"] == "SAFE"


def test_websocket_rejects_bad_auth(tmp_path):
    with TestClient(create_app(settings(tmp_path))) as client:
        try:
            with client.websocket_connect("/api/events") as websocket:
                websocket.send_json({"authorization": "Bearer bad"})
                websocket.receive_json()
        except Exception:
            pass
        else:
            raise AssertionError("unauthenticated websocket accepted")


def test_estop_endpoint_is_available_from_disconnected(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        result = client.post("/api/robot/estop", headers=headers).json()
        assert result["result"] == "accepted"
        assert result["target"] == "EMERGENCY_STOP"


def test_estop_publishes_estop_and_state_events(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        with client.websocket_connect("/api/events") as websocket:
            websocket.send_json({"authorization": f"Bearer {TOKEN}"})
            client.post("/api/robot/estop", headers=headers)
            first = websocket.receive_json()
            second = websocket.receive_json()
            assert first["type"] == "robot.estop"
            assert second["type"] == "robot.state"
            assert second["payload"]["state"] == "EMERGENCY_STOP"


def test_real_target_without_adapter_rejects_commands_and_stays_disconnected(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(real_settings(tmp_path))) as client:
        for target in ("SAFE", "MANUAL", "AUTONOMOUS"):
            response = client.post(
                "/api/robot/command", headers=headers,
                json={"target": target, "timeout": 1},
            )
            assert response.status_code == 503
            assert response.json()["detail"]["result"] == "rejected_real_target_unavailable"
        robot = client.get("/api/state", headers=headers).json()["robot"]
        assert robot["state"] == "DISCONNECTED"
        assert robot["target_mode"] == "real-target-unavailable"
        assert robot["authority"] == "unavailable"
        assert robot["execution"] == "not_delivered"


def test_real_target_without_adapter_never_claims_physical_event_or_estop(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(real_settings(tmp_path))) as client:
        command = client.post(
            "/api/robot/command", headers=headers,
            json={"target": "SAFE", "timeout": 1},
        )
        assert command.status_code == 503
        estop = client.post("/api/robot/estop", headers=headers)
        assert estop.status_code == 503
        assert estop.json()["detail"]["result"] == "requested_not_delivered"
        assert estop.json()["detail"]["result"] != "accepted"
        events = client.app.state.events.history()
        state_events = [event for event in events if event["type"] == "robot.state"]
        assert state_events
        assert all(event["payload"]["source"] == "real-target-unavailable" for event in state_events)
        assert all(event["payload"]["execution"] == "not_delivered" for event in state_events)
        assert all(event["payload"].get("source") != "physical" for event in state_events)


def test_real_target_without_adapter_is_unavailable_on_public_surfaces(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(real_settings(tmp_path))) as client:
        health_robot = client.get("/api/health", headers=headers).json()["robot"]
        assert health_robot["robot_state"] == "DISCONNECTED"
        assert health_robot["target_mode"] == "real-target-unavailable"
        assert health_robot["availability"] == "unavailable"
        assert health_robot["readiness"] == "not_ready"
        assert health_robot["execution"] == "not_delivered"
        assert health_robot["authority_generation"] == 0
        assert health_robot["session_id"] is None
        assert health_robot["physically_confirmed"] is False
        plugins = client.get("/api/plugins", headers=headers).json()["plugins"]
        robot_plugin = next(plugin for plugin in plugins if plugin["id"] == "robot")
        assert robot_plugin == {
            "id": "robot",
            "status": "real-target-unavailable",
            "permission": "emergency-request-only",
        }


def test_stt_oversized_input_is_rejected_before_worker_launch(tmp_path):
    called = False

    async def should_not_run(*args, **kwargs):
        nonlocal called
        called = True

    original = LocalTranscriber.transcribe_isolated
    LocalTranscriber.transcribe_isolated = should_not_run
    headers = {"Authorization": f"Bearer {TOKEN}"}
    try:
        with TestClient(create_app(settings(tmp_path))) as client:
            response = client.post(
                "/api/stt/transcribe", headers=headers,
                content=b"x" * (25 * 1024 * 1024 + 1),
            )
            assert response.status_code == 413
            assert called is False
    finally:
        LocalTranscriber.transcribe_isolated = original


def test_stt_worker_failure_isolated_from_main_app(tmp_path):
    async def failed_worker(*args, **kwargs):
        return {
            "success": False, "transcript": "", "provider": "local",
            "error": "STT_WORKER_CRASHED",
        }

    original = LocalTranscriber.transcribe_isolated
    LocalTranscriber.transcribe_isolated = failed_worker
    headers = {"Authorization": f"Bearer {TOKEN}"}
    try:
        with TestClient(create_app(settings(tmp_path))) as client:
            response = client.post("/api/stt/transcribe", headers=headers, content=b"audio")
            assert response.status_code == 422
            assert response.json()["detail"] == "STT_WORKER_CRASHED"
            assert client.get("/api/health", headers=headers).status_code == 200
    finally:
        LocalTranscriber.transcribe_isolated = original


def test_robot_command_domain_rejects_raw_or_unknown_targets(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        for target in ("GPIO", "PWM", "CAN_FRAME", "RAW_ACTUATOR", "future-state"):
            response = client.post(
                "/api/robot/command", headers=headers,
                json={"target": target, "timeout": 1},
            )
            assert response.status_code == 422
        assert not client.app.state.events.history()


def test_secrets_absent_from_status_events_and_audit(tmp_path, monkeypatch):
    secret = "sk-super-secret-provider-token-123456789"
    cfg = settings(tmp_path)
    cfg = Settings(
        cfg.host, cfg.port, cfg.auth_token, cfg.simulator, cfg.log_path,
        "hermes", cfg.hermes_session, secret, cfg.voice_id, None,
    )

    def factory(_key):
        def synth(text, voice_id, model_id, cancelled):
            yield b"audio"
        return synth

    monkeypatch.setattr("app.server.main.elevenlabs_synthesizer", factory)
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(cfg)) as client:
        payloads = [
            client.get("/api/health", headers=headers).json(),
            client.get("/api/settings", headers=headers).json(),
            client.get("/api/providers", headers=headers).json(),
            client.get("/api/voice/status", headers=headers).json(),
        ]
        client.post("/api/voice/speak", headers=headers, json={"text": secret})
        payloads.append(client.app.state.events.history())
    payloads.append(cfg.log_path.read_text(encoding="utf-8"))
    assert secret not in str(payloads)
    assert TOKEN not in str(payloads)


def test_static_hud_contains_every_required_panel(tmp_path):
    with TestClient(create_app(settings(tmp_path))) as client:
        html = client.get("/").text
        for panel in ("chat", "task", "robot", "vision", "computer", "terminal", "browser", "files", "memory", "system", "voice", "settings"):
            assert f'data-panel="{panel}"' in html


def test_settings_never_expose_secrets_and_voice_controls_work(tmp_path):
    cfg = settings(tmp_path)
    cfg = Settings(cfg.host, cfg.port, cfg.auth_token, cfg.simulator, cfg.log_path, "hermes", cfg.hermes_session, "cloud-secret", cfg.voice_id, "nanami")
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(cfg)) as client:
        body = client.get("/api/settings", headers=headers).json()
        serialized = str(body)
        assert TOKEN not in serialized and "cloud-secret" not in serialized
        assert body["voice_id"] == "voice-one"
        assert client.post("/api/voice/mute", headers=headers, json={"muted": True}).json()["result"] == "muted"
        assert client.post("/api/voice/stop", headers=headers).json()["result"] == "stopped"


def test_plugins_api_is_authenticated_and_secret_free(tmp_path):
    cfg = settings(tmp_path)
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(cfg)) as client:
        assert client.get("/api/plugins").status_code == 401
        response = client.get("/api/plugins", headers=headers)
        assert response.status_code == 200
        body = response.json()
        assert len(body["plugins"]) == 7
        robot = next(plugin for plugin in body["plugins"] if plugin["id"] == "robot")
        assert robot["status"] == "simulation"
        serialized = str(body)
        assert TOKEN not in serialized and "secret" not in serialized.lower()


def test_voice_speak_streams_chunk_safe_audio_and_publishes_events(tmp_path, monkeypatch):
    captured = []
    def factory(_key):
        def synth(text, voice_id, model_id, cancelled):
            captured.append((text, voice_id, model_id))
            yield b"ID3"
            yield b"audio"
        return synth
    monkeypatch.setattr("app.server.main.elevenlabs_synthesizer", factory)
    cfg = settings(tmp_path)
    cfg = Settings(cfg.host, cfg.port, cfg.auth_token, cfg.simulator, cfg.log_path,
                   None, cfg.hermes_session, "cloud-secret", cfg.voice_id, None)
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(cfg)) as client:
        response = client.post("/api/voice/speak", headers=headers,
                               json={"text": "say sk-abcdefghijklmnopqrstuvwxyz"})
        assert response.status_code == 200
        assert response.headers["content-type"].startswith("audio/mpeg")
        assert response.content == b"ID3audio"
        assert captured == [("say [REDACTED]", "voice-one", "eleven_multilingual_v2")]
        assert [e["type"] for e in client.app.state.events.history()[-2:]] == [
            "voice.speak", "voice.response"
        ]


def test_chat_unavailable_without_configured_hermes(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        response = client.post("/api/chat", headers=headers, json={"message": "hello"})
        assert response.status_code == 503


def test_chat_uses_bridge_and_publishes_route_tool_and_response_events(tmp_path, monkeypatch):
    cfg = settings(tmp_path)
    cfg = Settings(
        cfg.host, cfg.port, cfg.auth_token, cfg.simulator, cfg.log_path, "hermes",
        "stable-name", None, cfg.voice_id, None,
    )

    class FakeRoute:
        def public(self): return {"provider": "openai-codex", "model": "gpt-5.6-sol", "reason": "configured-primary"}

    class FakeBridge:
        last_route = FakeRoute()
        async def stream_bytes(self, message):
            assert message == "hello"
            yield b"world"

    monkeypatch.setattr("app.server.main.HermesBridge.executable", lambda *args, **kwargs: FakeBridge())
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(cfg)) as client:
        response = client.post("/api/chat", headers=headers, json={"message": "hello"})
        assert response.status_code == 200 and response.content == b"world"
        emitted = client.app.state.events.history()
        assert [event["type"] for event in emitted[-3:]] == [
            "chat.model_route", "chat.tool", "chat.response"
        ]
        assert emitted[-3]["payload"] == {
            "provider": "openai-codex", "model": "gpt-5.6-sol",
            "reason": "configured-primary",
        }
        assert emitted[-2]["payload"] == {"tool": "hermes-cli", "status": "started"}
        assert emitted[-1]["payload"]["bytes"] == 5
