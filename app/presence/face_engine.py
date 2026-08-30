from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
import random
import re


class FaceEngineMode(str, Enum):
    ONLINE_CONTROLLED = "ONLINE_CONTROLLED"
    OFFLINE_AUTONOMOUS = "OFFLINE_AUTONOMOUS"


class FaceState(str, Enum):
    IDLE = "IDLE"
    LISTENING = "LISTENING"
    THINKING = "THINKING"
    SPEAKING = "SPEAKING"
    SLEEPING = "SLEEPING"
    ALERT = "ALERT"
    ERROR = "ERROR"


@dataclass(frozen=True)
class FaceVariant:
    variant_id: str
    state: FaceState
    asset_refs: tuple[str, ...]
    weight: int = 1
    online_allowed: bool = True
    offline_allowed: bool = True


@dataclass(frozen=True)
class FacePack:
    pack_id: str
    version: str
    engine_compatibility: str
    content_hash: str
    license_id: str
    provenance: str
    storage_class: str
    variants: tuple[FaceVariant, ...]


@dataclass(frozen=True)
class FaceIntent:
    state: FaceState
    emotion: str = "NEUTRAL"
    intensity: float = 0.5
    gaze_x: float = 0.0
    gaze_y: float = 0.0


@dataclass(frozen=True)
class FaceSelection:
    mode: FaceEngineMode
    variant_id: str
    intent: FaceIntent
    autonomous_full_face_selection: bool


_IDENTIFIER = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$")
_HASH = re.compile(r"^sha256:[0-9a-f]{64}$")
_ASSET = re.compile(r"^asset\.[A-Za-z0-9][A-Za-z0-9._/-]{0,127}$")
_STORAGE = {"builtin", "internal", "downloaded-cache", "external-future"}


def validate_face_pack(pack: FacePack) -> None:
    """Reject unbounded, executable, ambiguous, or unlicensed face content."""
    if not _IDENTIFIER.fullmatch(pack.pack_id) or not _IDENTIFIER.fullmatch(pack.version):
        raise ValueError("invalid pack identity")
    if pack.engine_compatibility != "face-engine-v1":
        raise ValueError("incompatible face engine")
    if not _HASH.fullmatch(pack.content_hash):
        raise ValueError("invalid content hash")
    if not pack.license_id.strip() or not pack.provenance.strip():
        raise ValueError("license and provenance are required")
    if pack.storage_class not in _STORAGE:
        raise ValueError("unknown storage class")
    if not pack.variants or len(pack.variants) > 256:
        raise ValueError("invalid variant count")
    ids: set[str] = set()
    states: set[FaceState] = set()
    for variant in pack.variants:
        if not _IDENTIFIER.fullmatch(variant.variant_id) or variant.variant_id in ids:
            raise ValueError("invalid or duplicate variant")
        if variant.weight < 1 or variant.weight > 100:
            raise ValueError("invalid variant weight")
        if not variant.asset_refs or len(variant.asset_refs) > 16:
            raise ValueError("invalid asset count")
        if any(not _ASSET.fullmatch(ref) for ref in variant.asset_refs):
            raise ValueError("asset references must be bounded data handles")
        ids.add(variant.variant_id)
        states.add(variant.state)
    if FaceState.IDLE not in states:
        raise ValueError("IDLE fallback is required")


class FaceEngine:
    """Selects semantic face data; rendering and animation remain on ESP32."""

    def __init__(self, pack: FacePack, *, seed: int = 0):
        validate_face_pack(pack)
        self._pack = pack
        self._rng = random.Random(seed)
        self._mode = FaceEngineMode.OFFLINE_AUTONOMOUS
        self._last_variant: str | None = None

    @property
    def mode(self) -> FaceEngineMode:
        return self._mode

    def set_connectivity(self, *, host_available: bool, internet_available: bool) -> None:
        # Provider/Internet loss may inform offline personality, but an authoritative
        # Host presence connection alone determines whether host intent dominates.
        del internet_available
        self._mode = (
            FaceEngineMode.ONLINE_CONTROLLED
            if host_available
            else FaceEngineMode.OFFLINE_AUTONOMOUS
        )

    def select(self, intent: FaceIntent) -> FaceSelection:
        if not 0.0 <= intent.intensity <= 1.0:
            raise ValueError("intensity out of range")
        if not -1.0 <= intent.gaze_x <= 1.0 or not -1.0 <= intent.gaze_y <= 1.0:
            raise ValueError("gaze out of range")
        candidates = self._candidates(intent.state)
        if self._mode is FaceEngineMode.ONLINE_CONTROLLED:
            chosen = candidates[0]
            autonomous = False
        else:
            pool = [item for item in candidates if item.variant_id != self._last_variant]
            if not pool:
                pool = candidates
            chosen = self._rng.choices(pool, weights=[item.weight for item in pool], k=1)[0]
            autonomous = True
        self._last_variant = chosen.variant_id
        return FaceSelection(self._mode, chosen.variant_id, intent, autonomous)

    def _candidates(self, state: FaceState) -> list[FaceVariant]:
        online = self._mode is FaceEngineMode.ONLINE_CONTROLLED
        candidates = [
            item for item in self._pack.variants
            if item.state is state
            and (item.online_allowed if online else item.offline_allowed)
        ]
        if not candidates:
            candidates = [
                item for item in self._pack.variants
                if item.state is FaceState.IDLE
                and (item.online_allowed if online else item.offline_allowed)
            ]
        if not candidates:
            raise RuntimeError("face pack has no eligible fail-closed fallback")
        return candidates
