import asyncio
import json
import os
import time

from app.server.config import Settings, load_settings
from app.security.auth import BearerAuth
from app.security.redaction import redact, redact_text
from app.server.events import EventBus
from app.server.state import StateCore


def test_config_defaults_are_loopback_and_real_mode(monkeypatch):
    monkeypatch.delenv("ZIE_HOST", raising=False)
    monkeypatch.delenv("ZIE_SIMULATOR", raising=False)
    settings = load_settings({"ZIE_AUTH_TOKEN": "a" * 32})
    assert settings.host == "127.0.0.1"
    assert settings.simulator is False


def test_config_rejects_non_loopback_and_short_token():
    for values in (
        {"ZIE_HOST": "0.0.0.0", "ZIE_AUTH_TOKEN": "a" * 32},
        {"ZIE_AUTH_TOKEN": "short"},
    ):
        try:
            load_settings(values)
        except ValueError:
            pass
        else:
            raise AssertionError("unsafe configuration accepted")


def test_bearer_auth_uses_constant_time_validation():
    auth = BearerAuth("correct-token-with-enough-entropy")
    assert auth.valid("Bearer correct-token-with-enough-entropy")
    assert not auth.valid("Bearer wrong-token-with-enough-entropy")
    assert not auth.valid(None)


def test_recursive_redaction_and_cloud_text_redaction():
    value = {"token": "secret", "nested": {"voice_id": "allowed", "api_key": "key"}}
    assert redact(value) == {"token": "[REDACTED]", "nested": {"voice_id": "allowed", "api_key": "[REDACTED]"}}
    assert "[REDACTED]" in redact_text("say sk-test_abcdefghijklmnopqrstuvwxyz and Bearer abcdefghijklmnop")


def test_state_core_has_no_fake_robot_telemetry():
    state = StateCore(simulator=False).snapshot()
    assert state["robot"]["state"] == "DISCONNECTED"
    assert state["robot"]["telemetry"] is None
    assert state["robot"]["simulator"] is False


def test_event_bus_replays_ordered_bounded_events():
    async def scenario():
        bus = EventBus(history_size=2)
        await bus.publish("one", {"n": 1})
        await bus.publish("two", {"n": 2})
        await bus.publish("three", {"n": 3})
        events = bus.history()
        assert [event["type"] for event in events] == ["two", "three"]
        assert events[0]["id"] < events[1]["id"]
    asyncio.run(scenario())
