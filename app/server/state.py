from __future__ import annotations

from copy import deepcopy
from datetime import datetime, timezone
from threading import RLock
from typing import Any


class StateCore:
    def __init__(self, simulator: bool):
        self._lock = RLock()
        target_mode = "simulation" if simulator else "real-target-unavailable"
        self._state: dict[str, Any] = {
            "generation": 1,
            "updated_at": datetime.now(timezone.utc).isoformat(),
            "robot": {
                "state": "DISCONNECTED",
                "target": None,
                "telemetry": None,
                "simulator": simulator,
                "target_mode": target_mode,
                "authority": "simulation" if simulator else "unavailable",
                "execution": "simulated" if simulator else "not_delivered",
            },
            "hermes": {"connected": False, "session": None},
            "voice": {"speaking": False, "muted": False},
        }

    def snapshot(self) -> dict[str, Any]:
        with self._lock:
            return deepcopy(self._state)

    def update(self, section: str, values: dict[str, Any]) -> dict[str, Any]:
        with self._lock:
            self._state.setdefault(section, {}).update(deepcopy(values))
            self._state["generation"] += 1
            self._state["updated_at"] = datetime.now(timezone.utc).isoformat()
            return deepcopy(self._state)
