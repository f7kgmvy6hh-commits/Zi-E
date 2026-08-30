from __future__ import annotations

from dataclasses import dataclass
from enum import Enum


class LocalSafetyCommand(str, Enum):
    STOP = "STOP"
    FREEZE = "FREEZE"
    EMERGENCY_STOP = "EMERGENCY_STOP"


@dataclass(frozen=True)
class SafetyRecognition:
    command: LocalSafetyCommand
    event_id: str
    authority_granted: bool = False


class LocalSafetyVoiceRecognizer:
    """Fixed grammar classifier; dispatch and physical safety authority are external."""

    _GRAMMAR = {item.value: item for item in LocalSafetyCommand}

    def recognize(self, token: str, *, event_id: str) -> SafetyRecognition | None:
        normalized = "_".join(token.strip().upper().split())
        command = self._GRAMMAR.get(normalized)
        if command is None or not event_id:
            return None
        return SafetyRecognition(command=command, event_id=event_id)
