from __future__ import annotations

import asyncio
import csv
from dataclasses import asdict, dataclass
import importlib.util
import io
from pathlib import Path
import subprocess
import sys
from typing import Any

from app.security.redaction import redact_text


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
INVENTORY_TEMPLATE = REPOSITORY_ROOT / "docs" / "templates" / "PURCHASED_PARTS_INTAKE_TEMPLATE.csv"
PHASE2B2_STATE = "WAITING_FOR_VERIFIED_INPUTS"

APP_STATES = (
    "SIMULATED", "DISCONNECTED", "UNAVAILABLE", "NOT_CONFIGURED",
    "NOT_VERIFIED", "VERIFY_ON_ARRIVAL", "REQUESTED", "ACCEPTED",
    "DELIVERED", "PHYSICALLY_CONFIRMED", "FAULT", "COMMISSIONED",
)

WORKSPACES = (
    "OVERVIEW", "ROBOT", "CAMERA / VISION", "FACE / DISPLAY / RGB",
    "MOTORS / ACTUATORS", "SENSORS", "POWER / BATTERY", "CONTROLLERS",
    "VOICE / PROVIDERS", "HARDWARE INVENTORY", "P1 / CAD", "COMMISSIONING",
    "SAFETY", "PLUGINS / PROFILES", "DIAGNOSTICS", "DEVELOPER",
)

ACTUATORS = (
    "drive.left", "drive.right", "arm.left.shoulder", "arm.right.shoulder",
    "arm.left.elbow", "arm.right.elbow", "arm.left.wrist", "arm.right.wrist",
    "gripper.left", "gripper.right", "head.pan", "head.tilt", "head.lift",
    "arm.left.telescope", "arm.right.telescope",
)

SENSORS = (
    "range.head", "cliff.downward.*", "proximity.horizontal.*",
    "bumper.contact.*", "imu.body", "drive.encoder.*", "power.*",
)

COMMISSIONING_GATES = (
    "physical_inventory_reconciled", "mechanical_inspection",
    "continuity_short_check", "logic_only_power", "esp32_boot", "stm32_boot",
    "controller_identity_session", "communications", "sensors_one_by_one",
    "one_restrained_actuator", "direction_range_feedback",
    "safe_stop_verification", "display_audio_camera", "constrained_drivetrain",
    "integrated_restrained_test", "supervised_motion",
)


def _git(*arguments: str) -> str | None:
    try:
        result = subprocess.run(
            ["git", "-c", f"safe.directory={REPOSITORY_ROOT}", *arguments],
            cwd=REPOSITORY_ROOT, capture_output=True, text=True, timeout=3,
            check=False, shell=False,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    return result.stdout.strip() if result.returncode == 0 else None


def repository_status() -> dict[str, Any]:
    head = _git("rev-parse", "HEAD")
    short_head = _git("rev-parse", "--short", "HEAD")
    branch = _git("branch", "--show-current")
    porcelain = _git("status", "--short")
    recent = _git("log", "-5", "--pretty=format:%h %s")
    diff = _git("diff", "--stat")
    return {
        "path": str(REPOSITORY_ROOT),
        "available": head is not None,
        "branch": branch,
        "head": head,
        "short_head": short_head,
        "working_tree": "dirty" if porcelain else "clean" if porcelain is not None else "unavailable",
        "status": porcelain.splitlines() if porcelain else [],
        "diff_summary": diff.splitlines() if diff else [],
        "recent_log": recent.splitlines() if recent else [],
    }


def inventory_schema() -> list[str]:
    first = INVENTORY_TEMPLATE.read_text(encoding="utf-8").splitlines()[0]
    return next(csv.reader([first]))


def inventory_preview(content: bytes) -> dict[str, Any]:
    if not content:
        raise ValueError("inventory CSV is empty")
    if len(content) > 2 * 1024 * 1024:
        raise ValueError("inventory CSV exceeds 2 MiB")
    try:
        text = content.decode("utf-8-sig")
    except UnicodeDecodeError as exc:
        raise ValueError("inventory CSV must be UTF-8") from exc
    reader = csv.DictReader(io.StringIO(text))
    expected = inventory_schema()
    if reader.fieldnames != expected:
        raise ValueError("inventory CSV header does not match the canonical schema")
    rows: list[dict[str, Any]] = []
    for index, source in enumerate(reader, start=2):
        if len(rows) >= 500:
            raise ValueError("inventory CSV exceeds 500 rows")
        if None in source:
            raise ValueError(f"inventory CSV row {index} has unexpected columns")
        row = {key: (value or "").strip() for key, value in source.items()}
        row["row_number"] = index
        row["physical_verification"] = "NOT_VERIFIED"
        row["review_status"] = "REVIEW_REQUIRED"
        rows.append(row)
    return {
        "mode": "NON_AUTHORITATIVE_PREVIEW",
        "persisted": False,
        "physical_verification": "NOT_VERIFIED",
        "rows": rows,
        "count": len(rows),
    }


def cockpit_status(runtime: dict[str, Any], providers: dict[str, Any], stt: str, tts: str) -> dict[str, Any]:
    simulation = runtime["target_mode"] == "simulation"
    real_confirmed = (
        runtime["target_mode"] == "future-real-target"
        and runtime["physically_confirmed"] is True
    )
    mode = (
        "SIMULATED" if simulation else
        "FUTURE_REAL_TARGET" if runtime["target_mode"] == "future-real-target" else
        "DISCONNECTED"
    )
    hardware_state = "UNAVAILABLE"
    return {
        "state_vocabulary": APP_STATES,
        "workspaces": WORKSPACES,
        "target": {
            "mode": mode, "runtime": runtime, "physical_readiness": "NOT_VERIFIED",
            "execution_confirmation": "PHYSICALLY_CONFIRMED" if real_confirmed else "NOT_VERIFIED",
        },
        "controllers": [
            {"id": "esp32", "role": "multimedia/presence/display/camera/audio", "state": hardware_state,
             "identity": None, "firmware": None, "session_id": None, "heartbeat_age": None,
             "reset_reason": None, "faults": [], "commissioning": "NOT_VERIFIED"},
            {"id": "stm32", "role": "protected motion/safety/sensors/limits/faults", "state": hardware_state,
             "identity": None, "firmware": None, "session_id": None, "heartbeat_age": None,
             "reset_reason": None, "faults": [], "commissioning": "NOT_VERIFIED"},
        ],
        "camera": {"state": "UNAVAILABLE", "source": None, "resolution": None, "fps": None,
                   "latency_ms": None, "stream": None, "snapshot": "UNAVAILABLE", "overlays": "NOT_CONFIGURED"},
        "presentation": {"state": "NOT_CONFIGURED", "face_pack": None, "sound_pack": None,
                         "display_health": "UNAVAILABLE", "rgb_fail_state": "fail-dark"},
        "actuators": [
            {"slot": slot, "state": "UNAVAILABLE", "identity": None, "position": None,
             "commanded_position": None, "speed": None, "range": None, "current": None,
             "temperature": None, "fault": None, "commissioning": "NOT_VERIFIED",
             "controls": "semantic-bounded-future"} for slot in ACTUATORS
        ],
        "sensors": [
            {"slot": slot, "state": "UNAVAILABLE", "value": None, "units": None,
             "timestamp": None, "age": None, "fault": None, "commissioning": "NOT_VERIFIED",
             "verification": "VERIFY_ON_ARRIVAL"} for slot in SENSORS
        ],
        "power": {"state": "UNAVAILABLE", "pack_voltage": None, "battery_estimate": None,
                  "current": None, "charging": None, "rails": [], "bms": "NOT_VERIFIED",
                  "protection": "NOT_VERIFIED", "temperatures": [], "warnings": ["physical evidence required"]},
        "controller_link": {"state": "UNAVAILABLE", "phase2b2": PHASE2B2_STATE,
                            "bitrate": None, "rx_rate": None, "tx_rate": None,
                            "error_counters": None, "bus_off": None, "heartbeat": None,
                            "latency": None, "protocol_compatibility": "NOT_VERIFIED",
                            "transmit_api": False},
        "voice": {"stt": stt, "tts": tts, "providers": providers},
        "p1": {"documentation": "READY", "inventory": "NOT_VERIFIED", "commissioning": "NOT_DONE",
               "cad_version": "v0.3", "cad_checks": {"total": 39, "passed": 38, "restricted": 1},
               "cad_classes": ["FIXED", "PARAMETRIC", "BLOCKED_BY_MEASUREMENT", "VERIFY_ON_ARRIVAL"],
               "artifact_integrity": "WARNING", "first_power": "BLOCKED_PENDING_PHYSICAL_GATES",
               "phase2b2": PHASE2B2_STATE, "production_drivers": "NOT_STARTED"},
        "commissioning": {
            "state": "NOT_VERIFIED", "progress": 0,
            "virtual_robot_can_pass_physical_gate": False,
            "autonomous_physical_motion": "BLOCKED",
            "gates": [{"id": gate, "state": "NOT_VERIFIED", "requires": "reviewed_physical_evidence"}
                      for gate in COMMISSIONING_GATES],
        },
        "safety": {
            "motion_authority": "SIMULATION_ONLY" if simulation else "UNAVAILABLE",
            "physical_estop": "UNAVAILABLE", "safe_stop_verification": "NOT_VERIFIED",
            "lease": "UNAVAILABLE", "heartbeat": "UNAVAILABLE", "cliff": "NOT_VERIFIED",
            "bumper": "NOT_VERIFIED",
            "motion_blocked_reasons": [] if simulation else [
                "real target unavailable", "cliff sensors uncommissioned",
                "battery protection unverified", "controller link unavailable",
            ],
        },
        "firmware": {
            "esp32": {"detected": False, "firmware": None, "approved_target": None, "flash": "NOT_CONFIGURED"},
            "stm32": {"detected": False, "firmware": None, "approved_target": None, "flash": "NOT_CONFIGURED"},
            "unrestricted_flashing": False,
        },
    }


@dataclass(frozen=True)
class DeveloperAction:
    action_id: str
    title: str
    available: bool
    command: tuple[str, ...] | None
    timeout: int

    def public(self) -> dict[str, Any]:
        value = asdict(self)
        value.pop("command")
        return value


def developer_actions() -> dict[str, DeveloperAction]:
    cadquery_available = importlib.util.find_spec("cadquery") is not None
    return {
        "app_tests": DeveloperAction("app_tests", "Run full App pytest", True,
                                     (sys.executable, "-m", "pytest", "tests/app", "-q"), 180),
        "fallback_tests": DeveloperAction("fallback_tests", "Run fallback tests", True,
                                          (sys.executable, "scripts/run_tests_fallback.py"), 180),
        "compileall": DeveloperAction("compileall", "Compile Python App", True,
                                      (sys.executable, "-m", "compileall", "-q", "app", "tests/app"), 120),
        "cad_checks": DeveloperAction("cad_checks", "Run current CAD checks", cadquery_available,
                                      (sys.executable, "-B", "src/validate_design.py") if cadquery_available else None, 180),
        "open_repository": DeveloperAction("open_repository", "Open repository folder", False, None, 0),
        "open_terminal": DeveloperAction("open_terminal", "Open Windows Terminal at repository", False, None, 0),
        "launch_codex": DeveloperAction("launch_codex", "Launch Codex", False, None, 0),
        "launch_hermes": DeveloperAction("launch_hermes", "Launch Hermes", False, None, 0),
        "open_logs": DeveloperAction("open_logs", "Open App logs", False, None, 0),
    }


async def run_developer_action(action_id: str) -> dict[str, Any]:
    action = developer_actions().get(action_id)
    if action is None:
        raise KeyError("unknown developer action")
    if not action.available or action.command is None:
        raise RuntimeError("developer action is unavailable")
    cwd = REPOSITORY_ROOT / "mechanical" / "cad" / "current" if action_id == "cad_checks" else REPOSITORY_ROOT
    process = await asyncio.create_subprocess_exec(
        *action.command, cwd=cwd, stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.STDOUT,
    )
    try:
        output, _ = await asyncio.wait_for(process.communicate(), timeout=action.timeout)
    except asyncio.TimeoutError:
        process.kill()
        await process.communicate()
        return {"action_id": action_id, "state": "FAULT", "exit_code": None, "output": "timed out"}
    decoded = redact_text(output.decode("utf-8", errors="replace")[-20000:])
    return {
        "action_id": action_id,
        "state": "ACCEPTED" if process.returncode == 0 else "FAULT",
        "exit_code": process.returncode,
        "output": decoded,
        "robot_authority": "NONE",
    }
