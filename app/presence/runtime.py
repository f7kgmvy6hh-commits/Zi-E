from __future__ import annotations

from dataclasses import dataclass, replace
from enum import Enum


class PresenceState(str, Enum):
    BOOTING = "BOOTING"
    BOARD_UNVERIFIED = "BOARD_UNVERIFIED"
    NETWORK_PROVISIONING = "NETWORK_PROVISIONING"
    NETWORK_CONNECTING = "NETWORK_CONNECTING"
    HOST_CONNECTING = "HOST_CONNECTING"
    READY_IDLE = "READY_IDLE"
    LISTENING = "LISTENING"
    THINKING_REMOTE = "THINKING_REMOTE"
    SPEAKING = "SPEAKING"
    OFFLINE_LOCAL = "OFFLINE_LOCAL"
    DEGRADED = "DEGRADED"
    FAULTED = "FAULTED"
    SAFE_STOP_REQUESTED = "SAFE_STOP_REQUESTED"


class CapabilityState(str, Enum):
    AVAILABLE = "AVAILABLE"
    UNAVAILABLE = "UNAVAILABLE"
    UNVERIFIED = "UNVERIFIED"
    DISABLED = "DISABLED"


@dataclass(frozen=True)
class PresenceSnapshot:
    state: PresenceState = PresenceState.BOOTING
    board_verified: bool = False
    network_available: bool = False
    host_link_available: bool = False
    session_id: str | None = None
    generation_id: int = 0
    controller_link: CapabilityState = CapabilityState.UNVERIFIED
    fault: str | None = None


class PresenceRuntime:
    """Transport-independent ESP32 presence lifecycle reference model."""

    def __init__(self) -> None:
        self._snapshot = PresenceSnapshot()
        self._retired_sessions: set[str] = set()

    @property
    def snapshot(self) -> PresenceSnapshot:
        return self._snapshot

    def board_evaluated(self, *, verified: bool) -> PresenceSnapshot:
        self._snapshot = replace(
            self._snapshot,
            board_verified=verified,
            state=PresenceState.NETWORK_CONNECTING if verified else PresenceState.BOARD_UNVERIFIED,
        )
        return self._snapshot

    def network_changed(self, available: bool) -> PresenceSnapshot:
        if not available:
            self._retire_session()
            state = PresenceState.OFFLINE_LOCAL if self._snapshot.board_verified else PresenceState.BOARD_UNVERIFIED
        else:
            state = PresenceState.HOST_CONNECTING
        self._snapshot = replace(self._snapshot, network_available=available, state=state)
        return self._snapshot

    def host_connected(self, session_id: str, generation_id: int) -> PresenceSnapshot:
        if not self._snapshot.network_available or not self._snapshot.board_verified:
            raise ValueError("verified board and network are required")
        if not session_id or len(session_id) > 64 or generation_id <= self._snapshot.generation_id:
            raise ValueError("fresh bounded session and generation are required")
        if session_id in self._retired_sessions:
            raise ValueError("retired session cannot reactivate")
        self._retire_session()
        self._snapshot = replace(
            self._snapshot, host_link_available=True, session_id=session_id,
            generation_id=generation_id, state=PresenceState.READY_IDLE, fault=None,
        )
        return self._snapshot

    def host_disconnected(self) -> PresenceSnapshot:
        self._retire_session()
        self._snapshot = replace(
            self._snapshot, host_link_available=False, session_id=None,
            state=PresenceState.OFFLINE_LOCAL if self._snapshot.board_verified else PresenceState.BOARD_UNVERIFIED,
        )
        return self._snapshot

    def request_safe_stop(self) -> PresenceSnapshot:
        self._snapshot = replace(self._snapshot, state=PresenceState.SAFE_STOP_REQUESTED)
        return self._snapshot

    def _retire_session(self) -> None:
        if self._snapshot.session_id:
            self._retired_sessions.add(self._snapshot.session_id)


def unverified_presence_status() -> dict[str, object]:
    return {
        "state": PresenceState.BOARD_UNVERIFIED.value,
        "source": "INVENTORY_EVIDENCE",
        "truth": "NOT_VERIFIED",
        "host_link": "UNAVAILABLE",
        "network": "UNAVAILABLE",
        "session_id": None,
        "generation_id": 0,
        "capabilities": {
            name: CapabilityState.UNVERIFIED.value for name in (
                "audio.capture", "audio.playback", "audio.opus", "wake.local",
                "camera.capture", "face.render", "face.pack", "rgb.effects",
                "safety_voice.local", "controller_link.semantic",
            )
        },
        "physical_authority": False,
    }
