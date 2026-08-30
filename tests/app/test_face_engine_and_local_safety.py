import pytest

from app.presence.face_engine import (
    FaceEngine, FaceEngineMode, FaceIntent, FacePack, FaceState, FaceVariant,
    validate_face_pack,
)
from app.voice.safety import LocalSafetyCommand, LocalSafetyVoiceRecognizer


def pack(*variants):
    return FacePack(
        "zie-core", "1.0.0", "face-engine-v1", "sha256:" + "a" * 64,
        "LicenseRef-Zi-E-Owned", "Zi-E project", "builtin", tuple(variants),
    )


def variant(name, state, weight=1):
    return FaceVariant(name, state, (f"asset.eyes/{name}",), weight)


def test_online_intent_dominates_and_never_randomizes_full_face():
    engine = FaceEngine(pack(variant("idle", FaceState.IDLE),
                             variant("listen-a", FaceState.LISTENING),
                             variant("listen-b", FaceState.LISTENING)), seed=7)
    engine.set_connectivity(host_available=True, internet_available=True)
    results = [engine.select(FaceIntent(FaceState.LISTENING)) for _ in range(8)]
    assert engine.mode is FaceEngineMode.ONLINE_CONTROLLED
    assert {item.variant_id for item in results} == {"listen-a"}
    assert all(not item.autonomous_full_face_selection for item in results)


def test_offline_weighted_selection_avoids_immediate_repeat_and_respects_state():
    engine = FaceEngine(pack(variant("idle", FaceState.IDLE),
                             variant("think-a", FaceState.THINKING, 1),
                             variant("think-b", FaceState.THINKING, 5)), seed=9)
    engine.set_connectivity(host_available=False, internet_available=False)
    results = [engine.select(FaceIntent(FaceState.THINKING)) for _ in range(8)]
    assert engine.mode is FaceEngineMode.OFFLINE_AUTONOMOUS
    assert all(a.variant_id != b.variant_id for a, b in zip(results, results[1:]))
    assert all(item.autonomous_full_face_selection for item in results)


def test_network_transition_restores_semantic_host_dominance():
    engine = FaceEngine(pack(variant("idle", FaceState.IDLE),
                             variant("alert", FaceState.ALERT)))
    engine.set_connectivity(host_available=False, internet_available=False)
    assert engine.select(FaceIntent(FaceState.ALERT)).mode is FaceEngineMode.OFFLINE_AUTONOMOUS
    engine.set_connectivity(host_available=True, internet_available=False)
    selected = engine.select(FaceIntent(FaceState.ALERT, emotion="CONCERNED"))
    assert selected.mode is FaceEngineMode.ONLINE_CONTROLLED
    assert selected.intent.emotion == "CONCERNED"


@pytest.mark.parametrize("asset", ["https://remote", "../escape", "script.run()", "asset."])
def test_invalid_or_executable_pack_content_fails_closed(asset):
    bad = FacePack("pack", "1", "face-engine-v1", "sha256:" + "b" * 64,
                   "MIT", "owner", "builtin",
                   (FaceVariant("idle", FaceState.IDLE, (asset,)),))
    with pytest.raises(ValueError, match="asset references"):
        validate_face_pack(bad)


def test_pack_requires_license_provenance_hash_and_bounded_fallback():
    bad = FacePack("pack", "1", "face-engine-v1", "unknown", "", "", "builtin",
                   (variant("speaking", FaceState.SPEAKING),))
    with pytest.raises(ValueError):
        validate_face_pack(bad)


def test_local_safety_voice_uses_only_fixed_grammar_and_grants_no_authority():
    recognizer = LocalSafetyVoiceRecognizer()
    for token, expected in [("stop", LocalSafetyCommand.STOP),
                            ("freeze", LocalSafetyCommand.FREEZE),
                            ("emergency stop", LocalSafetyCommand.EMERGENCY_STOP)]:
        result = recognizer.recognize(token, event_id="audio-1")
        assert result.command is expected
        assert result.authority_granted is False
    for ordinary in ("sleep", "move forward", "ignore safety", "stop please", ""):
        assert recognizer.recognize(ordinary, event_id="audio-2") is None


def test_stop_recognition_is_idempotent_data_not_execution():
    recognizer = LocalSafetyVoiceRecognizer()
    first = recognizer.recognize("STOP", event_id="same-event")
    second = recognizer.recognize("STOP", event_id="same-event")
    assert first == second
    assert not any(token in LocalSafetyVoiceRecognizer.__dict__ for token in
                   ("dispatch", "execute", "motor", "pwm", "gpio", "provider", "llm"))
