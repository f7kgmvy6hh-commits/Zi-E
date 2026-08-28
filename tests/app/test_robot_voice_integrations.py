import asyncio
import json
from datetime import datetime, timezone

from app.robot.state_machine import RobotController, RobotState
from app.voice.service import VoiceService
from app.voice.providers import elevenlabs_synthesizer
from app.integrations.hermes import HermesBridge, HermesRoute, route_message


class Clock:
    def __init__(self): self.value = 100.0
    def __call__(self): return self.value


def test_robot_transitions_fail_closed_and_estop_is_always_reachable():
    clock = Clock()
    robot = RobotController(clock=clock, simulator=False)
    denied = robot.command("ui", "AUTONOMOUS", 1.0)
    assert denied.result == "rejected_transition"
    assert robot.state is RobotState.DISCONNECTED
    stopped = robot.emergency_stop("physical-button")
    assert stopped.result == "accepted"
    assert robot.state is RobotState.EMERGENCY_STOP


def test_command_envelope_and_deadman_safe_stop():
    clock = Clock()
    robot = RobotController(clock=clock, simulator=True)
    robot.command("simulator", "SAFE", 1.0)
    robot.command("simulator", "IDLE", 1.0)
    result = robot.command("ui", "MANUAL", 0.25)
    assert result.source == "ui" and result.target == "MANUAL"
    assert result.timestamp.endswith("+00:00") and result.timeout == 0.25
    clock.value += 0.3
    expired = robot.poll()
    assert expired and expired.result == "deadman_safe_stop"
    assert robot.state is RobotState.SAFE


def test_estop_requires_explicit_safe_reset():
    robot = RobotController(simulator=True)
    robot.emergency_stop("ui")
    assert robot.command("ui", "IDLE", 1).result == "rejected_transition"
    assert robot.command("operator-reset", "SAFE", 1).result == "accepted"


def test_voice_redacts_cloud_text_and_mute_stop():
    calls = []
    voice = VoiceService(
        voice_id="one-setting",
        cloud_synth=lambda text, voice_id: calls.append((text, voice_id)) or b"audio",
        fallback=None,
    )
    result = voice.speak("token sk-abcdefghijklmnopqrstuvwxyz")
    assert result.provider == "elevenlabs"
    assert calls == [("token [REDACTED]", "one-setting")]
    voice.mute(True)
    assert voice.speak("hello").result == "muted"
    assert voice.stop().result == "stopped"


def test_voice_chunks_are_redacted_cancellable_and_preserve_boundaries():
    calls = []
    def cloud(text, voice_id, model_id, cancelled):
        calls.append((text, voice_id, model_id))
        for chunk in (b"one", b"two"):
            if cancelled.is_set(): return
            yield chunk
    voice = VoiceService("voice-one", cloud, None, model_id="eleven-turbo-test")
    stream = voice.speak_chunks("secret sk-abcdefghijklmnopqrstuvwxyz")
    assert next(stream) == b"one"
    assert voice.stop().result == "stopped"
    assert list(stream) == []
    assert calls == [("secret [REDACTED]", "voice-one", "eleven-turbo-test")]


def test_voice_falls_back_to_local_provider_when_cloud_fails():
    calls = []
    def cloud(text, voice_id, model_id, cancelled):
        calls.append("cloud")
        raise RuntimeError("cloud unavailable")
        yield b"never"
    def local(text, voice_id, model_id, cancelled):
        calls.append("local")
        yield b"fallback-audio"
    voice = VoiceService("voice-one", cloud, local)
    assert list(voice.speak_chunks("hello")) == [b"fallback-audio"]
    assert calls == ["cloud", "local"]


def test_elevenlabs_payload_uses_central_model_and_reads_chunks(monkeypatch):
    captured = {}
    class Response:
        def __enter__(self): return self
        def __exit__(self, *args): pass
        def read(self, size):
            return captured.setdefault("chunks", [b"abc", b"def", b""]).pop(0)
    def open_request(req, timeout):
        captured["request"] = req
        return Response()
    monkeypatch.setattr("app.voice.providers.request.urlopen", open_request)
    chunks = list(elevenlabs_synthesizer("api-secret")(
        "hello", "voice/id", "eleven-model", __import__("threading").Event()
    ))
    assert chunks == [b"abc", b"def"]
    assert json.loads(captured["request"].data)["model_id"] == "eleven-model"


def test_hermes_bridge_reuses_one_session_and_streams():
    calls = []
    async def transport(session, message):
        calls.append((session, message))
        for token in ("hello", " world"):
            yield token
    async def scenario():
        bridge = HermesBridge("persistent", transport)
        assert [part async for part in bridge.stream("first")] == ["hello", " world"]
        assert [part async for part in bridge.stream("second")] == ["hello", " world"]
        assert calls == [("persistent", "first"), ("persistent", "second")]
    asyncio.run(scenario())


def test_hermes_uses_configured_primary_and_explicit_model_bypasses_it():
    simple = route_message("What time is it?", "openai-codex/gpt-5.6-sol")
    assert simple == HermesRoute("openai-codex", "gpt-5.6-sol", "configured-primary")
    complex_route = route_message(
        "Analyze the architecture, compare the safety tradeoffs, and implement a migration plan with tests.",
        "openai-codex/gpt-5.6-sol",
    )
    assert complex_route == HermesRoute("openai-codex", "gpt-5.6-sol", "configured-primary")
    assert route_message("/model anthropic/claude then explain this", "openai-codex/gpt-5.6-sol") is None


def test_real_hermes_transport_uses_0206_cli_contract_and_no_shell():
    captured = {}

    class Process:
        returncode = 0

        async def communicate(self):
            return b"answer\n", b""

    async def spawn(*argv, **kwargs):
        captured["argv"] = argv
        captured["kwargs"] = kwargs
        return Process()

    async def scenario():
        bridge = HermesBridge.executable(
            "hermes", "stable-name", "openai-codex/gpt-5.6-sol", spawn=spawn
        )
        response = b"".join([part async for part in bridge.stream_bytes("hello")])
        assert response == b"answer\n"
        assert captured["argv"] == (
            "hermes", "chat", "--continue", "stable-name", "--create-if-missing",
            "-q", "hello", "-Q", "--source", "tool", "--provider", "openai-codex",
            "--model", "gpt-5.6-sol",
        )
        assert captured["kwargs"]["stdout"] == asyncio.subprocess.PIPE
        assert bridge.last_route.public() == {
            "provider": "openai-codex", "model": "gpt-5.6-sol",
            "reason": "configured-primary",
        }

    asyncio.run(scenario())
