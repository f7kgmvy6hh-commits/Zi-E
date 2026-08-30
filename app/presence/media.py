from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from collections import deque


class MediaCapability(str, Enum):
    AVAILABLE = "AVAILABLE"
    UNAVAILABLE = "UNAVAILABLE"
    UNVERIFIED = "UNVERIFIED"
    DISABLED = "DISABLED"


class CameraFormat(str, Enum):
    JPEG = "JPEG"
    RGB565 = "RGB565"


MAX_CAMERA_TRANSFER_BYTES = 2 * 1024 * 1024


@dataclass(frozen=True)
class AudioFrame:
    session_id: str
    sequence: int
    payload: bytes


class BoundedAudioQueue:
    def __init__(self, maximum_frames: int = 100, maximum_frame_bytes: int = 4096):
        if not 1 <= maximum_frames <= 500 or not 1 <= maximum_frame_bytes <= 16384:
            raise ValueError("invalid audio bounds")
        self._frames: deque[AudioFrame] = deque()
        self.maximum_frames = maximum_frames
        self.maximum_frame_bytes = maximum_frame_bytes
        self.dropped_frames = 0
        self.underflows = 0

    def push(self, frame: AudioFrame) -> bool:
        if not frame.session_id or frame.sequence < 0 or not frame.payload or len(frame.payload) > self.maximum_frame_bytes:
            return False
        if len(self._frames) >= self.maximum_frames:
            self.dropped_frames += 1
            return False
        self._frames.append(frame)
        return True

    def pop(self) -> AudioFrame | None:
        if not self._frames:
            self.underflows += 1
            return None
        return self._frames.popleft()

    def reset_session(self) -> None:
        self._frames.clear()

    def __len__(self) -> int:
        return len(self._frames)


@dataclass(frozen=True)
class CameraCaptureRequest:
    request_id: str
    accepted_formats: tuple[CameraFormat, ...]
    maximum_bytes: int = MAX_CAMERA_TRANSFER_BYTES

    def validate(self) -> None:
        if not self.request_id or not self.accepted_formats:
            raise ValueError("invalid capture request")
        if not 1 <= self.maximum_bytes <= MAX_CAMERA_TRANSFER_BYTES:
            raise ValueError("capture transfer limit rejected")


def media_capability_status() -> dict[str, object]:
    return {
        "audio": {"capture": "UNVERIFIED", "playback": "UNVERIFIED", "opus": "UNAVAILABLE",
                  "aec": "UNVERIFIED", "nr": "UNVERIFIED", "agc": "UNVERIFIED"},
        "wake": {"state": "UNVERIFIED", "provider_owned": False},
        "camera": {"state": "UNAVAILABLE", "source": "NO_PHYSICAL_CAMERA",
                   "maximum_transfer_bytes": MAX_CAMERA_TRANSFER_BYTES},
        "display": {"state": "UNVERIFIED", "renderer": "MODEL_ONLY"},
        "rgb": {"state": "UNVERIFIED", "raw_control_exposed": False},
    }
