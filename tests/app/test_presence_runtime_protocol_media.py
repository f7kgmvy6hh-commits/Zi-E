import json

import pytest

from app.presence.media import AudioFrame, BoundedAudioQueue, CameraCaptureRequest, CameraFormat
from app.presence.protocol import (
    BoundedRequestLedger, MAX_CONTROL_BYTES, MessageType, RequestState,
    decode_control_message,
)
from app.presence.runtime import PresenceRuntime, PresenceState


def envelope(**changes):
    value = {
        "protocol_major": 1, "protocol_minor": 0, "message_type": "FACE_INTENT",
        "session_id": "session-2", "generation_id": 2, "request_id": "request-1",
        "payload": {"state": "LISTENING"},
    }
    value.update(changes)
    return json.dumps(value).encode()


def test_presence_requires_verified_board_and_fresh_session():
    runtime = PresenceRuntime()
    assert runtime.board_evaluated(verified=False).state is PresenceState.BOARD_UNVERIFIED
    with pytest.raises(ValueError):
        runtime.host_connected("session-1", 1)
    runtime.board_evaluated(verified=True)
    runtime.network_changed(True)
    assert runtime.host_connected("session-1", 1).state is PresenceState.READY_IDLE
    runtime.host_disconnected()
    with pytest.raises(ValueError, match="retired"):
        runtime.host_connected("session-1", 2)


def test_internet_is_not_presence_authority_and_link_loss_retires_session():
    runtime = PresenceRuntime()
    runtime.board_evaluated(verified=True)
    runtime.network_changed(True)
    runtime.host_connected("local-lan", 1)
    assert runtime.snapshot.host_link_available
    assert runtime.host_disconnected().state is PresenceState.OFFLINE_LOCAL


def test_control_envelope_accepts_only_active_bounded_semantics():
    decoded = decode_control_message(envelope(), active_session="session-2", active_generation=2)
    assert decoded.message_type is MessageType.FACE_INTENT
    for bad in (
        envelope(protocol_major=2), envelope(message_type="RAW_PWM"),
        envelope(session_id="stale"), envelope(generation_id=1), b"not-json",
        b"{" + b"x" * MAX_CONTROL_BYTES,
    ):
        with pytest.raises(ValueError):
            decode_control_message(bad, active_session="session-2", active_generation=2)


@pytest.mark.parametrize("payload", [
    {"path": "../secret"}, {"path": "C:\\secret"}, {"items": list(range(65))},
    {"text": "x" * 257},
])
def test_control_payload_rejects_paths_and_unbounded_values(payload):
    with pytest.raises(ValueError):
        decode_control_message(envelope(payload=payload), active_session="session-2", active_generation=2)


def test_audio_queue_is_bounded_drops_newest_and_clears_on_session_reset():
    queue = BoundedAudioQueue(maximum_frames=2, maximum_frame_bytes=8)
    assert queue.push(AudioFrame("s", 1, b"one"))
    assert queue.push(AudioFrame("s", 2, b"two"))
    assert not queue.push(AudioFrame("s", 3, b"three"))
    assert queue.dropped_frames == 1 and len(queue) == 2
    queue.reset_session()
    assert len(queue) == 0 and queue.pop() is None and queue.underflows == 1


def test_camera_control_request_has_transfer_limit_and_no_image_payload():
    request = CameraCaptureRequest("capture-1", (CameraFormat.JPEG,), 100_000)
    request.validate()
    with pytest.raises(ValueError):
        CameraCaptureRequest("capture-2", (CameraFormat.JPEG,), 3_000_000).validate()


def test_request_ledger_bounds_duplicates_and_cancellation_without_execution_claim():
    ledger = BoundedRequestLedger(2)
    assert ledger.receive("request-1") == (RequestState.RECEIVED, False)
    assert ledger.receive("request-1") == (RequestState.RECEIVED, True)
    assert ledger.cancel("request-1") is RequestState.CANCELLED
    assert ledger.cancel("request-1") is RequestState.CANCELLED
    ledger.reset_session()
    with pytest.raises(ValueError, match="unknown"):
        ledger.cancel("request-1")
