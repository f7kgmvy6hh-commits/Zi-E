from __future__ import annotations

from dataclasses import asdict, dataclass
from enum import Enum
from typing import Protocol, runtime_checkable

from app.robot.state_machine import CommandResult, RobotController


class TargetMode(str, Enum):
    SIMULATION = "simulation"
    REAL_TARGET_UNAVAILABLE = "real-target-unavailable"
    FUTURE_REAL_TARGET = "future-real-target"


class RequestPhase(str, Enum):
    REQUESTED = "requested"
    ACCEPTED = "accepted"
    REJECTED = "rejected"
    CONFIRMED = "confirmed"


@dataclass(frozen=True)
class SemanticCommandRequest:
    target: str
    timeout: float
    source: str = "hud"


@dataclass(frozen=True)
class SemanticCommandResponse:
    source: str
    timestamp: str
    target: str
    timeout: float
    result: str
    phase: RequestPhase
    target_mode: TargetMode
    authority_generation: int
    delivered: bool
    physically_confirmed: bool

    def public(self) -> dict[str, object]:
        value = asdict(self)
        value["phase"] = self.phase.value
        value["target_mode"] = self.target_mode.value
        return value


@dataclass(frozen=True)
class RuntimeStatus:
    target_mode: TargetMode
    availability: str
    readiness: str
    robot_state: str
    authority_generation: int
    session_id: str | None
    execution: str
    physically_confirmed: bool

    def public(self) -> dict[str, object]:
        value = asdict(self)
        value["target_mode"] = self.target_mode.value
        return value


@runtime_checkable
class HostRuntimeAdapter(Protocol):
    """Narrow App boundary. Implementations expose semantic services only."""

    def status(self) -> RuntimeStatus: ...
    def submit(self, request: SemanticCommandRequest) -> SemanticCommandResponse: ...
    def request_estop(self, source: str) -> SemanticCommandResponse: ...
    def poll(self) -> SemanticCommandResponse | None: ...


def _response(
    result: CommandResult,
    *,
    phase: RequestPhase,
    target_mode: TargetMode,
    generation: int,
    delivered: bool,
    confirmed: bool = False,
) -> SemanticCommandResponse:
    return SemanticCommandResponse(
        source=result.source,
        timestamp=result.timestamp,
        target=result.target,
        timeout=result.timeout,
        result=result.result,
        phase=phase,
        target_mode=target_mode,
        authority_generation=generation,
        delivered=delivered,
        physically_confirmed=confirmed,
    )


class SimulatorRuntimeAdapter:
    def __init__(self, controller: RobotController | None = None):
        self.controller = controller or RobotController(simulator=True)
        if not self.controller.simulator:
            raise ValueError("simulator adapter requires a simulator controller")

    def status(self) -> RuntimeStatus:
        return RuntimeStatus(
            TargetMode.SIMULATION, "available", "ready", self.controller.state.value,
            1, "simulation-session", "simulated", False,
        )

    def submit(self, request: SemanticCommandRequest) -> SemanticCommandResponse:
        result = self.controller.command(request.source, request.target, request.timeout)
        phase = RequestPhase.REJECTED if result.result.startswith("rejected") else RequestPhase.ACCEPTED
        return _response(
            result, phase=phase, target_mode=TargetMode.SIMULATION, generation=1,
            delivered=result.result == "accepted",
        )

    def request_estop(self, source: str) -> SemanticCommandResponse:
        return _response(
            self.controller.emergency_stop(source), phase=RequestPhase.ACCEPTED,
            target_mode=TargetMode.SIMULATION, generation=1, delivered=True,
        )

    def poll(self) -> SemanticCommandResponse | None:
        result = self.controller.poll()
        if result is None:
            return None
        return _response(
            result, phase=RequestPhase.ACCEPTED, target_mode=TargetMode.SIMULATION,
            generation=1, delivered=True,
        )


class UnavailableRuntimeAdapter:
    def __init__(self, controller: RobotController | None = None):
        self.controller = controller or RobotController(simulator=False)
        if self.controller.simulator:
            raise ValueError("unavailable adapter requires a non-simulator controller")

    def status(self) -> RuntimeStatus:
        return RuntimeStatus(
            TargetMode.REAL_TARGET_UNAVAILABLE, "unavailable", "not_ready",
            self.controller.state.value, 0, None, "not_delivered", False,
        )

    def submit(self, request: SemanticCommandRequest) -> SemanticCommandResponse:
        result = self.controller.command(request.source, request.target, request.timeout)
        return _response(
            result, phase=RequestPhase.REJECTED,
            target_mode=TargetMode.REAL_TARGET_UNAVAILABLE, generation=0,
            delivered=False,
        )

    def request_estop(self, source: str) -> SemanticCommandResponse:
        return _response(
            self.controller.emergency_stop(source), phase=RequestPhase.REQUESTED,
            target_mode=TargetMode.REAL_TARGET_UNAVAILABLE, generation=0,
            delivered=False,
        )

    def poll(self) -> SemanticCommandResponse | None:
        return None

