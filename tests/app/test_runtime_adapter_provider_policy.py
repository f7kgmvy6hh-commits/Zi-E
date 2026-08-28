import pytest
import time
from fastapi.testclient import TestClient

from app.providers.policy import ExactModalityChain, Modality, ProviderBinding, ProviderStatus
from app.runtime.adapter import RequestPhase, RuntimeStatus, SemanticCommandResponse, TargetMode
from app.server.main import create_app
from tests.app.test_server import TOKEN, real_settings, settings


def binding(provider, modality, capability, priority, invoke, recheck):
    return ProviderBinding(
        ProviderStatus(provider, modality, capability, priority, True, "not_verified", "healthy"),
        invoke,
        recheck,
    )


def test_provider_fallback_is_exact_bounded_and_primary_is_retried():
    calls = []
    primary_healthy = [False]
    primary = binding(
        "openai-stt", Modality.STT, "provider.invoke.stt", 1,
        lambda request: calls.append("primary") or "primary-result",
        lambda: (True, "healthy" if primary_healthy[0] else "unavailable"),
    )
    fallback = binding(
        "local-whisper", Modality.STT, "provider.invoke.stt", 2,
        lambda request: calls.append("fallback") or "fallback-result",
        lambda: (True, "healthy"),
    )
    chain = ExactModalityChain(Modality.STT, "provider.invoke.stt", [fallback, primary])

    first = chain.invoke(b"audio")
    assert first.provider_id == "local-whisper" and calls == ["fallback"]
    primary_healthy[0] = True
    second = chain.invoke(b"audio")
    assert second.provider_id == "openai-stt" and calls == ["fallback", "primary"]


def test_provider_chain_rejects_capability_or_modality_widening():
    wrong = binding(
        "tts-provider", Modality.TTS, "provider.invoke.tts", 1,
        lambda request: b"audio", lambda: (True, "healthy"),
    )
    with pytest.raises(ValueError, match="mismatch"):
        ExactModalityChain(Modality.STT, "provider.invoke.stt", [wrong])


class StaleRuntimeAdapter:
    def __init__(self):
        self.generation = 7
        self.session = "runtime-session-7"
        self.revoked = False

    def status(self):
        return RuntimeStatus(
            TargetMode.FUTURE_REAL_TARGET,
            "unavailable" if self.revoked else "available",
            "not_ready" if self.revoked else "ready",
            "DISCONNECTED" if self.revoked else "SAFE",
            self.generation,
            self.session,
            "not_delivered" if self.revoked else "confirmed",
            False,
        )

    def submit(self, request):
        before = self.status()
        self.revoked = True
        self.generation += 1
        self.session = "runtime-session-8"
        return SemanticCommandResponse(
            request.source, "2026-08-28T00:00:00+00:00", request.target,
            request.timeout, "accepted", RequestPhase.ACCEPTED,
            TargetMode.FUTURE_REAL_TARGET, before.authority_generation, True, False,
        )

    def request_estop(self, source):
        status = self.status()
        return SemanticCommandResponse(
            source, "2026-08-28T00:00:00+00:00", "EMERGENCY_STOP", 0,
            "requested_not_delivered", RequestPhase.REQUESTED,
            TargetMode.FUTURE_REAL_TARGET, status.authority_generation, False, False,
        )

    def poll(self):
        return None


def test_stale_adapter_generation_cannot_revive_command_authority(tmp_path):
    adapter = StaleRuntimeAdapter()
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(real_settings(tmp_path), runtime_adapter=adapter)) as client:
        response = client.post(
            "/api/robot/command", headers=headers,
            json={"target": "SAFE", "timeout": 1},
        )
        assert response.status_code == 503
        assert response.json()["detail"] == "runtime authority changed during command"
        assert not client.app.state.events.history()


def test_config_flag_cannot_install_simulation_adapter_as_real_authority(tmp_path):
    from app.runtime.adapter import SimulatorRuntimeAdapter

    with pytest.raises(ValueError, match="cannot use simulation authority"):
        create_app(real_settings(tmp_path), runtime_adapter=SimulatorRuntimeAdapter())


def test_restart_starts_from_safe_non_authoritative_state(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    for _ in range(2):
        with TestClient(create_app(real_settings(tmp_path))) as client:
            robot = client.get("/api/state", headers=headers).json()["robot"]
            assert robot["state"] == "DISCONNECTED"
            assert robot["authority"] == "unavailable"
            assert robot["physically_confirmed"] is False


def test_optional_provider_unavailability_cannot_mask_robot_blocker(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(real_settings(tmp_path))) as client:
        health = client.get("/api/health", headers=headers).json()
        assert health["status"] == "blocked"
        assert health["readiness"]["required_robot"] == "unavailable"
        assert health["readiness"]["overall"] == "blocked"
        assert all(not entry["configured"] for entry in health["providers"]["tts"])


def test_public_foundation_surfaces_are_honest_and_read_only(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path))) as client:
        assert client.get("/api/hardware-profile", headers=headers).json()["profile_id"] is None
        assert client.get("/api/configuration", headers=headers).json()["mutable"] is False
        assert client.get("/api/presentation", headers=headers).json()["context"] is None
        providers = client.get("/api/providers", headers=headers).json()["modalities"]
        assert set(providers) == {"llm", "stt", "tts", "wake"}
        for modality, entries in providers.items():
            priorities = [entry["priority"] for entry in entries]
            assert priorities == sorted(priorities)
            assert all(entry["modality"] == modality for entry in entries)
            assert all(entry["authorization"] == "not_verified" for entry in entries)


def test_runtime_adapter_contract_has_no_raw_control_surface():
    from app.runtime.adapter import HostRuntimeAdapter

    names = {name.lower() for name in HostRuntimeAdapter.__dict__}
    forbidden = ("gpio", "pwm", "register", "can", "driver", "actuator", "raw")
    assert not any(token in name for name in names for token in forbidden)


class FixedRuntimeAdapter:
    def __init__(self, response):
        self.response = response

    def status(self):
        return RuntimeStatus(
            TargetMode.FUTURE_REAL_TARGET, "available", "ready", "SAFE", 9,
            "runtime-session-9", "confirmed", self.response.physically_confirmed,
        )

    def submit(self, request):
        return self.response

    def request_estop(self, source):
        return self.response

    def poll(self):
        return None


def future_response(*, phase, delivered, confirmed):
    return SemanticCommandResponse(
        "hud", "2026-08-28T00:00:00+00:00", "SAFE", 1, phase.value,
        phase, TargetMode.FUTURE_REAL_TARGET, 9, delivered, confirmed,
    )


def test_future_physical_confirmation_must_be_consistent_authoritative_feedback(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    invalid = future_response(phase=RequestPhase.ACCEPTED, delivered=True, confirmed=True)
    with TestClient(create_app(real_settings(tmp_path), FixedRuntimeAdapter(invalid))) as client:
        response = client.post(
            "/api/robot/command", headers=headers,
            json={"target": "SAFE", "timeout": 1},
        )
        assert response.status_code == 503
        assert not client.app.state.events.history()

    confirmed = future_response(phase=RequestPhase.CONFIRMED, delivered=True, confirmed=True)
    with TestClient(create_app(real_settings(tmp_path), FixedRuntimeAdapter(confirmed))) as client:
        response = client.post(
            "/api/robot/command", headers=headers,
            json={"target": "SAFE", "timeout": 1},
        )
        assert response.status_code == 200
        assert response.json()["physically_confirmed"] is True
        state_events = [
            event for event in client.app.state.events.history()
            if event["type"] == "robot.state"
        ]
        assert state_events[-1]["payload"]["source"] == "future-real-target"
        assert state_events[-1]["payload"]["physically_confirmed"] is True


@pytest.mark.parametrize(
    ("phase", "delivered", "confirmed"),
    [
        (RequestPhase.REQUESTED, False, False),
        (RequestPhase.REJECTED, True, False),
        (RequestPhase.CONFIRMED, True, False),
        (RequestPhase.ACCEPTED, False, False),
    ],
)
def test_inconsistent_typed_command_outcomes_fail_before_events(
    tmp_path, phase, delivered, confirmed,
):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    response_value = future_response(
        phase=phase, delivered=delivered, confirmed=confirmed,
    )
    with TestClient(create_app(real_settings(tmp_path), FixedRuntimeAdapter(response_value))) as client:
        response = client.post(
            "/api/robot/command", headers=headers,
            json={"target": "SAFE", "timeout": 1},
        )
        assert response.status_code == 503
        assert not client.app.state.events.history()


class StalePollingAdapter(StaleRuntimeAdapter):
    def __init__(self):
        super().__init__()
        self.polled = False

    def poll(self):
        if self.polled:
            return None
        self.polled = True
        before = self.status()
        self.revoked = True
        self.generation += 1
        self.session = "runtime-session-8"
        return SemanticCommandResponse(
            "deadman", "2026-08-28T00:00:00+00:00", "SAFE", 0,
            "confirmed", RequestPhase.CONFIRMED, TargetMode.FUTURE_REAL_TARGET,
            before.authority_generation, True, True,
        )


def test_stale_polled_feedback_cannot_publish_execution(tmp_path):
    adapter = StalePollingAdapter()
    with TestClient(create_app(real_settings(tmp_path), adapter)) as client:
        time.sleep(0.12)
        events = client.app.state.events.history()
        assert not any(event["type"] == "robot.deadman" for event in events)
        rejected = [event for event in events if event["type"] == "runtime.feedback_rejected"]
        assert rejected and rejected[-1]["payload"]["reason"] == "stale_or_inconsistent_authority"


def test_estop_http_status_uses_typed_delivery_not_result_string(tmp_path):
    response_value = SemanticCommandResponse(
        "hud", "2026-08-28T00:00:00+00:00", "EMERGENCY_STOP", 0,
        "custom_text", RequestPhase.REQUESTED, TargetMode.FUTURE_REAL_TARGET,
        9, False, False,
    )
    adapter = FixedRuntimeAdapter(response_value)
    adapter.status = lambda: RuntimeStatus(
        TargetMode.FUTURE_REAL_TARGET, "available", "ready", "SAFE", 9,
        "runtime-session-9", "not_delivered", False,
    )
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(real_settings(tmp_path), adapter)) as client:
        response = client.post("/api/robot/estop", headers=headers)
        assert response.status_code == 503
        assert response.json()["detail"]["phase"] == "requested"
