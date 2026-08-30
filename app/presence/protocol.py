from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
import json
import re
from typing import Any
from collections import deque


MAX_CONTROL_BYTES = 16 * 1024
MAX_STRING = 256
MAX_ARRAY = 64
PROTOCOL_MAJOR = 1


class MessageType(str, Enum):
    CAPABILITIES = "CAPABILITIES"
    FACE_INTENT = "FACE_INTENT"
    AUDIO_SESSION_START = "AUDIO_SESSION_START"
    AUDIO_SESSION_STOP = "AUDIO_SESSION_STOP"
    CAMERA_CAPTURE_REQUEST = "CAMERA_CAPTURE_REQUEST"
    CANCEL_REQUEST = "CANCEL_REQUEST"
    ROBOT_INTENT = "ROBOT_INTENT"
    STOP_REQUEST = "STOP_REQUEST"
    DEVICE_EVENT = "DEVICE_EVENT"
    EXECUTION_EVENT = "EXECUTION_EVENT"
    FAULT_EVENT = "FAULT_EVENT"


_ID = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._:-]{0,63}$")


@dataclass(frozen=True)
class SemanticEnvelope:
    protocol_major: int
    protocol_minor: int
    message_type: MessageType
    session_id: str
    generation_id: int
    request_id: str
    payload: dict[str, Any]


class RequestState(str, Enum):
    RECEIVED = "RECEIVED"
    ACCEPTED = "ACCEPTED"
    EXECUTING = "EXECUTING"
    COMPLETED = "COMPLETED"
    REJECTED = "REJECTED"
    CANCELLED = "CANCELLED"
    FAULTED = "FAULTED"


class BoundedRequestLedger:
    """Tracks correlation only; ACCEPTED never implies physical completion."""

    def __init__(self, maximum_requests: int = 128):
        if not 1 <= maximum_requests <= 1024:
            raise ValueError("invalid request ledger bound")
        self.maximum_requests = maximum_requests
        self._states: dict[str, RequestState] = {}
        self._order: deque[str] = deque()

    def receive(self, request_id: str) -> tuple[RequestState, bool]:
        if not _ID.fullmatch(request_id):
            raise ValueError("malformed request identity")
        if request_id in self._states:
            return self._states[request_id], True
        if len(self._order) == self.maximum_requests:
            expired = self._order.popleft()
            del self._states[expired]
        self._order.append(request_id)
        self._states[request_id] = RequestState.RECEIVED
        return RequestState.RECEIVED, False

    def cancel(self, request_id: str) -> RequestState:
        state = self._states.get(request_id)
        if state is None:
            raise ValueError("unknown cancellation target")
        if state in {RequestState.COMPLETED, RequestState.REJECTED,
                     RequestState.CANCELLED, RequestState.FAULTED}:
            return state
        self._states[request_id] = RequestState.CANCELLED
        return RequestState.CANCELLED

    def reset_session(self) -> None:
        self._states.clear()
        self._order.clear()


def decode_control_message(data: bytes, *, active_session: str, active_generation: int) -> SemanticEnvelope:
    if not data or len(data) > MAX_CONTROL_BYTES:
        raise ValueError("control message size rejected")
    try:
        raw = json.loads(data)
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ValueError("invalid control JSON") from exc
    if not isinstance(raw, dict) or set(raw) != {
        "protocol_major", "protocol_minor", "message_type", "session_id",
        "generation_id", "request_id", "payload",
    }:
        raise ValueError("invalid envelope fields")
    if raw["protocol_major"] != PROTOCOL_MAJOR or not isinstance(raw["protocol_minor"], int):
        raise ValueError("unsupported protocol version")
    try:
        message_type = MessageType(raw["message_type"])
    except (ValueError, TypeError) as exc:
        raise ValueError("unknown message type") from exc
    for key in ("session_id", "request_id"):
        if not isinstance(raw[key], str) or not _ID.fullmatch(raw[key]):
            raise ValueError("malformed identity")
    if raw["session_id"] != active_session or raw["generation_id"] != active_generation:
        raise ValueError("stale session or generation")
    _validate_value(raw["payload"], depth=0)
    return SemanticEnvelope(
        raw["protocol_major"], raw["protocol_minor"], message_type,
        raw["session_id"], raw["generation_id"], raw["request_id"], raw["payload"],
    )


def _validate_value(value: Any, *, depth: int) -> None:
    if depth > 6:
        raise ValueError("payload nesting rejected")
    if isinstance(value, str):
        if (len(value) > MAX_STRING or value.startswith(("/", "\\"))
                or re.match(r"^[A-Za-z]:[\\/]", value) or ".." in value):
            raise ValueError("unsafe or oversized string")
    elif isinstance(value, list):
        if len(value) > MAX_ARRAY:
            raise ValueError("oversized array")
        for item in value:
            _validate_value(item, depth=depth + 1)
    elif isinstance(value, dict):
        if len(value) > MAX_ARRAY:
            raise ValueError("oversized object")
        for key, item in value.items():
            if not isinstance(key, str) or not _ID.fullmatch(key):
                raise ValueError("invalid payload key")
            _validate_value(item, depth=depth + 1)
    elif value is not None and not isinstance(value, (bool, int, float)):
        raise ValueError("unsupported payload value")
