from __future__ import annotations

from dataclasses import asdict, dataclass
from datetime import datetime, timezone
from enum import Enum
import time
from typing import Callable


class RobotState(str, Enum):
    DISCONNECTED = "DISCONNECTED"
    SAFE = "SAFE"
    IDLE = "IDLE"
    MANUAL = "MANUAL"
    ASSISTED = "ASSISTED"
    AUTONOMOUS = "AUTONOMOUS"
    PAUSED = "PAUSED"
    FAULT = "FAULT"
    EMERGENCY_STOP = "EMERGENCY_STOP"


@dataclass(frozen=True)
class CommandResult:
    source: str
    timestamp: str
    target: str
    timeout: float
    result: str

    def as_dict(self):
        return asdict(self)


_TRANSITIONS = {
    RobotState.DISCONNECTED: {RobotState.SAFE},
    RobotState.SAFE: {RobotState.IDLE, RobotState.DISCONNECTED},
    RobotState.IDLE: {RobotState.SAFE, RobotState.MANUAL, RobotState.ASSISTED, RobotState.AUTONOMOUS},
    RobotState.MANUAL: {RobotState.SAFE, RobotState.IDLE, RobotState.PAUSED},
    RobotState.ASSISTED: {RobotState.SAFE, RobotState.IDLE, RobotState.PAUSED},
    RobotState.AUTONOMOUS: {RobotState.SAFE, RobotState.IDLE, RobotState.PAUSED},
    RobotState.PAUSED: {RobotState.SAFE, RobotState.IDLE, RobotState.MANUAL, RobotState.ASSISTED, RobotState.AUTONOMOUS},
    RobotState.FAULT: {RobotState.SAFE, RobotState.DISCONNECTED},
    RobotState.EMERGENCY_STOP: {RobotState.SAFE},
}


class RobotController:
    def __init__(self, clock: Callable[[], float] = time.monotonic, simulator: bool = False):
        self._clock = clock
        self.simulator = simulator
        self.state = RobotState.DISCONNECTED
        self._deadline: float | None = None
        self.last_command: CommandResult | None = None

    def _result(self, source: str, target: RobotState, timeout: float, result: str) -> CommandResult:
        value = CommandResult(source, datetime.now(timezone.utc).isoformat(), target.value, timeout, result)
        self.last_command = value
        return value

    def command(self, source: str, target: str, timeout: float) -> CommandResult:
        try:
            desired = RobotState(target)
        except ValueError:
            return self._result(source, RobotState.SAFE, timeout, "rejected_unknown_target")
        if not source or timeout <= 0 or timeout > 30:
            return self._result(source or "unknown", desired, timeout, "rejected_invalid_command")
        if desired is RobotState.EMERGENCY_STOP:
            return self.emergency_stop(source)
        if not self.simulator:
            return self._result(source, desired, timeout, "rejected_real_target_unavailable")
        if desired not in _TRANSITIONS[self.state]:
            return self._result(source, desired, timeout, "rejected_transition")
        self.state = desired
        self._deadline = self._clock() + timeout if desired in {RobotState.MANUAL, RobotState.ASSISTED, RobotState.AUTONOMOUS} else None
        return self._result(source, desired, timeout, "accepted")

    def emergency_stop(self, source: str) -> CommandResult:
        if not self.simulator:
            self._deadline = None
            return self._result(
                source or "unknown", RobotState.EMERGENCY_STOP, 0.0,
                "requested_not_delivered",
            )
        self.state = RobotState.EMERGENCY_STOP
        self._deadline = None
        return self._result(source or "unknown", RobotState.EMERGENCY_STOP, 0.0, "accepted")

    def poll(self) -> CommandResult | None:
        if self._deadline is None or self._clock() < self._deadline:
            return None
        self.state = RobotState.SAFE
        self._deadline = None
        return self._result("deadman", RobotState.SAFE, 0.0, "deadman_safe_stop")
