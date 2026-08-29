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


def test_static_hud_contains_every_control_center_workspace(tmp_path):
    with TestClient(create_app(settings(tmp_path))) as client:
        html = client.get("/").text
        for workspace in (
            "overview", "robot", "camera", "face", "motors", "sensors",
            "power", "controllers", "voice", "inventory", "p1",
            "commissioning", "safety", "plugins", "diagnostics", "developer",
        ):
            assert f'data-w="{workspace}"' in html


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


def test_control_center_exposes_all_workspaces_and_honest_physical_state(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        body = client.get("/api/control-center", headers=headers).json()
        assert len(body["workspaces"]) == 16
        assert body["target"]["mode"] == "SIMULATED"
        assert body["target"]["physical_readiness"] == "NOT_VERIFIED"
        assert body["commissioning"]["progress"] == 0
        assert body["commissioning"]["virtual_robot_can_pass_physical_gate"] is False
        assert body["commissioning"]["autonomous_physical_motion"] == "BLOCKED"
        assert all(gate["state"] == "NOT_VERIFIED" for gate in body["commissioning"]["gates"])
        assert body["controller_link"]["phase2b2"] == "WAITING_FOR_VERIFIED_INPUTS"
        assert body["controller_link"]["transmit_api"] is False


def test_real_target_control_center_remains_disconnected_and_safety_blocked(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(real_settings(tmp_path))) as client:
        body = client.get("/api/control-center", headers=headers).json()
        assert body["target"]["mode"] == "DISCONNECTED"
        assert body["safety"]["motion_authority"] == "UNAVAILABLE"
        assert body["safety"]["physical_estop"] == "UNAVAILABLE"
        assert "real target unavailable" in body["safety"]["motion_blocked_reasons"]
        assert all(controller["state"] == "UNAVAILABLE" for controller in body["controllers"])
        assert body["target"]["execution_confirmation"] == "NOT_VERIFIED"


def test_inventory_preview_is_non_authoritative_and_never_self_verifies(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        schema = client.get("/api/hardware/inventory/schema", headers=headers).json()
        row = {field: "" for field in schema["columns"]}
        row.update({"inventory_id": "P1-001", "part_name": "user-entered part"})
        import csv
        import io
        stream = io.StringIO()
        writer = csv.DictWriter(stream, fieldnames=schema["columns"])
        writer.writeheader()
        writer.writerow(row)
        response = client.post(
            "/api/hardware/inventory/preview", headers=headers,
            content=stream.getvalue().encode("utf-8"),
        )
        assert response.status_code == 200
        body = response.json()
        assert body["persisted"] is False
        assert body["physical_verification"] == "NOT_VERIFIED"
        assert body["rows"][0]["physical_verification"] == "NOT_VERIFIED"
        assert body["rows"][0]["review_status"] == "REVIEW_REQUIRED"
        malformed = stream.getvalue().splitlines()[0] + "\n" + ",".join(
            ["value"] * (len(schema["columns"]) + 1)
        )
        assert client.post(
            "/api/hardware/inventory/preview", headers=headers,
            content=malformed.encode("utf-8"),
        ).status_code == 422


def test_developer_workspace_is_allowlisted_and_has_no_robot_or_shell_authority(tmp_path, monkeypatch):
    async def fixed_failure(action_id, *_):
        if action_id != "compileall":
            raise KeyError("unknown developer action")
        return {
            "action_id": action_id, "state": "FAULT", "exit_code": 7,
            "output": "fixed test failure", "robot_authority": "NONE",
        }

    monkeypatch.setattr("app.server.main.run_developer_action", fixed_failure)
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        before = client.get("/api/runtime", headers=headers).json()
        body = client.get("/api/developer", headers=headers).json()
        assert body["arbitrary_commands"] is False
        assert body["filesystem_api"] is False
        assert body["robot_authority"] == "NONE"
        assert client.post(
            "/api/developer/action", headers=headers,
            json={"action_id": "arbitrary_command"},
        ).status_code == 404
        action = client.post(
            "/api/developer/action", headers=headers,
            json={"action_id": "compileall"},
        )
        assert action.status_code == 200
        assert action.json()["robot_authority"] == "NONE"
        assert action.json()["state"] == "FAULT"
        assert client.get("/api/runtime", headers=headers).json() == before
        assert not any(
            event["type"].startswith("robot.")
            for event in client.app.state.events.history()
        )
        paths = {route.path for route in client.app.routes}
        assert "/terminal" not in paths
        assert "/api/terminal" not in paths
        assert not any("gpio" in path.lower() or "pwm" in path.lower() or "can/transmit" in path.lower() for path in paths)


def test_semantic_contracts_disable_raw_motion_presentation_and_flashing(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        body = client.get("/api/semantic-contracts", headers=headers).json()
        assert body["motion"]["state"] == "NOT_CONFIGURED"
        assert body["motion"]["raw_motor_control"] is False
        assert body["actuator_commissioning"]["raw_actuator_control"] is False
        assert body["presentation"]["arbitrary_display_memory"] is False
        assert body["firmware"]["unrestricted_flash"] is False
        html = client.get("/").text
        for label in ("OVERVIEW", "CAMERA / VISION", "HARDWARE INVENTORY", "COMMISSIONING", "DEVELOPER"):
            assert label in html
        assert "setInterval(()=>{if(held)sendDrive(selectedDirection,true)},250)" in html
        assert "onpointercancel=releaseDrive" in html
        assert "inventory-blockers').onclick" in html
        assert "(x.measurement_requirements||[]).join(', ')" in html
        assert "(x.commissioning_dependencies||[]).join(', ')" in html
        assert "ZI-E 0.02" not in html


def test_current_version_and_functional_preview_surfaces_are_honest(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        assert client.get("/api/health", headers=headers).json()["version"] == "0.04"
        cockpit = client.get("/api/control-center", headers=headers).json()
        assert cockpit["version"] == "0.04"
        assert cockpit["camera"]["mode"] == "TEST_SOURCE"
        assert cockpit["camera"]["fps"] == 0
        assert cockpit["camera"]["source"] == "generated-static-frame"
        assert cockpit["power"]["battery_estimate"] is None
        assert cockpit["controller_link"]["phase2b2"] == "WAITING_FOR_VERIFIED_INPUTS"
        assert "physical commissioning gates incomplete" in cockpit["safety"]["motion_blocked_reasons"]


def test_preview_semantics_never_physically_confirm_and_reject_extra_fields(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        drive = client.post("/api/drive/preview", headers=headers,
            json={"direction": "forward", "speed_limit": 20, "deadman": True}).json()
        assert drive["delivery"] == "SIMULATED" and drive["physically_confirmed"] is False
        presentation = client.post("/api/presentation/preview", headers=headers, json={
            "expression": "happy", "face_pack": "builtin-preview", "brightness": 50,
            "animation_speed": 50, "rgb_brightness": 30, "rgb_pattern": "calm",
            "display_test_pattern": False,
        }).json()
        assert presentation["delivery"] == "SIMULATED"
        assert presentation["physically_confirmed"] is False
        assert client.post("/api/drive/preview", headers=headers, json={
            "direction": "stop", "speed_limit": 0, "deadman": False, "command": "whoami",
        }).status_code == 422


def test_single_actuator_commissioning_preview_and_real_fail_closed(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        first = client.post("/api/actuators/commissioning-preview", headers=headers,
            json={"slot": "head.pan", "operation": "enable", "speed": 10})
        assert first.json()["physically_confirmed"] is False
        assert client.post("/api/actuators/commissioning-preview", headers=headers,
            json={"slot": "head.tilt", "operation": "enable", "speed": 10}).status_code == 409
    with TestClient(create_app(real_settings(tmp_path))) as client:
        result = client.post("/api/actuators/commissioning-preview", headers=headers,
            json={"slot": "head.pan", "operation": "enable", "speed": 10}).json()
        assert result["state"] == "REJECTED" and result["delivery"] == "NOT_DELIVERED"
        assert result["physically_confirmed"] is False


def test_developer_request_has_no_arbitrary_arguments(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        assert client.post("/api/developer/action", headers=headers,
            json={"action_id": "compileall", "arguments": ["--evil"]}).status_code == 422
