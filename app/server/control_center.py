from __future__ import annotations

import asyncio
from copy import deepcopy
import csv
from dataclasses import asdict, dataclass
from functools import lru_cache
import importlib.util
import io
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
from threading import RLock
import time
from typing import Any

from app.security.redaction import redact_text
from app.version import APP_STAGE, APP_VERSION
from app.presence.media import media_capability_status
from app.presence.runtime import unverified_presence_status


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
INVENTORY_TEMPLATE = REPOSITORY_ROOT / "docs" / "templates" / "PURCHASED_PARTS_INTAKE_TEMPLATE.csv"
PHASE2B2_STATE = "WAITING_FOR_VERIFIED_INPUTS"
ESP32_BUILD_STATUS = REPOSITORY_ROOT / "runtime" / "esp32-generic-build.json"
PROJECT_EVIDENCE = REPOSITORY_ROOT / "docs" / "evidence" / "ZI-E_PROJECT_EVIDENCE.json"
REPOSITORY_STATUS_TTL_SECONDS = 2.0
_repository_cache_lock = RLock()
_repository_cache: tuple[float, dict[str, Any]] | None = None

APP_STATES = (
    "SIMULATED", "DISCONNECTED", "UNAVAILABLE", "NOT_CONFIGURED",
    "NOT_VERIFIED", "VERIFY_ON_ARRIVAL", "REQUESTED", "ACCEPTED",
    "DELIVERED", "PHYSICALLY_CONFIRMED", "FAULT", "COMMISSIONED",
)
EXPRESSIONS = ("neutral", "happy", "curious", "sleepy", "warning")
DRIVE_DIRECTIONS = ("forward", "backward", "left", "right", "stop")
CAMERA_MODES = ("UNAVAILABLE", "TEST_SOURCE", "SIMULATED", "FUTURE_PHYSICAL")

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
            ["git", "-c", f"safe.directory={REPOSITORY_ROOT.as_posix()}", *arguments],
            cwd=REPOSITORY_ROOT, capture_output=True, text=True, timeout=3,
            check=False, shell=False,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    return result.stdout.strip() if result.returncode == 0 else None


def _repository_status_uncached() -> dict[str, Any]:
    porcelain = _git("status", "--short", "--branch")
    head = _git("rev-parse", "HEAD")
    recent = _git("log", "-5", "--pretty=format:%h %s")
    diff = _git("diff", "--stat")
    status_lines = porcelain.splitlines() if porcelain is not None else []
    branch_line = status_lines[0] if status_lines and status_lines[0].startswith("## ") else ""
    branch = branch_line[3:].split("...")[0].strip() if branch_line else None
    if branch == "HEAD (no branch)":
        branch = None
    changes = status_lines[1:] if branch_line else status_lines
    return {
        "path": str(REPOSITORY_ROOT),
        "available": head is not None,
        "branch": branch,
        "head": head,
        "short_head": head[:7] if head else None,
        "working_tree": "dirty" if changes else "clean" if porcelain is not None else "unavailable",
        "status": changes,
        "diff_summary": diff.splitlines() if diff else [],
        "recent_log": recent.splitlines() if recent else [],
    }


def clear_repository_status_cache() -> None:
    global _repository_cache
    with _repository_cache_lock:
        _repository_cache = None


def repository_status() -> dict[str, Any]:
    global _repository_cache
    now = time.monotonic()
    with _repository_cache_lock:
        if _repository_cache and now - _repository_cache[0] < REPOSITORY_STATUS_TTL_SECONDS:
            return deepcopy(_repository_cache[1])
        value = _repository_status_uncached()
        _repository_cache = (now, value)
        return deepcopy(value)


_EVIDENCE_IDS = {"SPARE-HW678", "HW-002", "HW-006", "HW-007", "HW-008", "HW-010", "HW-017"}
_EXPECTED_HARDWARE_STATES = {
    "SPARE-HW678": ("PRESENT", "INDEPENDENT_BENCH_VERIFIED", "BENCH_PASS", "NOT_HW_002"),
    "HW-002": ("NOT_ARRIVED", "UNVERIFIED", "NOT_TESTED", "BLOCKED_PENDING_ARRIVAL"),
    "HW-006": ("PRESENT", "INDEPENDENT_BENCH_PASS", "POWER_BIAS_ADC_ACOUSTIC_GAIN_PASS", "NOT_COMMISSIONED"),
    "HW-007": ("PRESENT", "IDENTITY_VERIFIED", "RANGING_NOT_VERIFIED", "QUARANTINED"),
    "HW-008": ("PRESENT", "INDEPENDENT_BENCH_PASS", "DI_RGB_CHANNELS_3V3_330OHM_PASS", "NOT_COMMISSIONED_FINAL_5V_NOT_VERIFIED"),
    "HW-010": ("PRESENT", "UNVERIFIED", "NOT_TESTED", "DO_NOT_POWER_OR_CONNECT"),
    "HW-017": ("PRESENT", "BENCH_TOOL_OBSERVED", "IN_USE", "BENCH_TOOL"),
}
_EXPECTED_HARDWARE_LABELS = {
    "SPARE-HW678": "spare HW678 ESP32-S3 development board",
    "HW-002": "final camera / Presence board",
    "HW-006": "MAX9814 microphone module",
    "HW-007": "VL53L0X range sensor",
    "HW-008": "WS2812-family RGB breakout",
    "HW-010": "unverified power board",
    "HW-017": "digital multimeter",
}
_EXPECTED_HARDWARE_RESTRICTIONS = {
    "SPARE-HW678": {"TEMPORARY_BENCH_PINS_NOT_PRODUCTION", "5VIN_NOT_VERIFIED_5V_OUTPUT"},
    "HW-002": {"NO_BOARD_PROFILE", "NO_GPIO_BINDING", "NO_FLASH"},
    "HW-006": {"TEMPORARY_GPIO4_NOT_PRODUCTION", "NO_FINAL_AUDIO_QUALITY_CLAIM"},
    "HW-007": {"DO_NOT_POWER_IN_THIS_PASS", "REWORK_OR_REPLACEMENT_REQUIRED", "TEMPORARY_GPIO5_GPIO6_NOT_PRODUCTION"},
    "HW-008": {"TEMPORARY_GPIO7_NOT_PRODUCTION", "TEMPORARY_3V3_BENCH_OPERATION_ONLY",
               "FINAL_5V_OPERATION_NOT_VERIFIED", "DO_NOT_USE_HW678_5VIN_AS_5V_SOURCE",
               "INTEGRATED_RGB_NOT_COMMISSIONED"},
    "HW-010": {"IDENTITY_POLARITY_IO_ISOLATION_CURRENT_PROTECTION_UNKNOWN"},
    "HW-017": {"EXACT_SAFETY_CATEGORY_UNVERIFIED"},
}


def _invalid_project_evidence() -> dict[str, Any]:
    return {
        "schema": "zie.project-evidence.v1", "evidence_status": "INVALID_UNAVAILABLE",
        "software": {
            "host_development": "UNKNOWN", "esp32_toolchain": "UNKNOWN",
            "esp32_generic_cross_build": "NOT_RUN", "esp32_target": "ESP32-S3",
            "esp32_profile": "GENERIC_UNVERIFIED_ESP32S3",
            "esp32_physical_target": "HW_002_NOT_ARRIVED_UNVERIFIED",
            "esp32_flash": "NOT_AUTHORIZED", "stm32_development": "INCOMPLETE",
            "physical_commissioning": "NOT_STARTED", "first_integrated_power": "NOT_AUTHORIZED",
            "phase2b2": PHASE2B2_STATE,
        }, "hardware": [],
    }


def _read_project_evidence(path: str, modified_ns: int, size: int) -> dict[str, Any]:
    del modified_ns, size
    fallback = _invalid_project_evidence()
    try:
        value = json.loads(Path(path).read_text(encoding="utf-8"))
        if not isinstance(value, dict):
            return fallback
        software = value.get("software")
        hardware = value.get("hardware")
        software_fields = {"host_development", "esp32_toolchain", "esp32_generic_cross_build",
            "esp32_target", "esp32_profile", "esp32_physical_target", "esp32_flash",
            "stm32_development", "physical_commissioning", "first_integrated_power", "phase2b2"}
        if (set(value) != {"schema", "updated", "software", "hardware"}
                or value.get("schema") != "zie.project-evidence.v1"
                or not re.fullmatch(r"\d{4}-\d{2}-\d{2}", str(value.get("updated")))
                or not isinstance(software, dict)
                or set(software) != software_fields
                or any(not isinstance(item, str) or len(item) > 64 for item in software.values())
                or not isinstance(hardware, list) or len(hardware) != len(_EVIDENCE_IDS)
                or {item.get("id") for item in hardware if isinstance(item, dict)} != _EVIDENCE_IDS
                or software.get("host_development") not in {"READY", "INCOMPLETE", "UNKNOWN"}
                or software.get("esp32_toolchain") not in {"READY", "INCOMPLETE", "UNKNOWN"}
                or software.get("esp32_generic_cross_build") not in {"PASS", "FAIL", "NOT_RUN"}
                or software.get("stm32_development") not in {"READY", "INCOMPLETE", "UNKNOWN"}
                or software.get("esp32_target") != "ESP32-S3"
                or software.get("esp32_profile") != "GENERIC_UNVERIFIED_ESP32S3"
                or software.get("esp32_flash") != "NOT_AUTHORIZED"
                or software.get("phase2b2") != PHASE2B2_STATE
                or software.get("physical_commissioning") != "NOT_STARTED"
                or software.get("first_integrated_power") != "NOT_AUTHORIZED"
                or software.get("esp32_physical_target") != "HW_002_NOT_ARRIVED_UNVERIFIED"):
            return fallback
        required = {"id", "label", "physical_state", "evidence_state", "functional_state",
                    "integration_state", "restrictions"}
        if any(set(item) != required or not isinstance(item["restrictions"], list)
               or len(item["restrictions"]) > 8
               or any(not isinstance(item[key], str) or not item[key] or len(item[key]) > 96
                      for key in required - {"restrictions"})
               or any(not isinstance(entry, str) or len(entry) > 96 for entry in item["restrictions"])
               for item in hardware):
            return fallback
        by_id = {item["id"]: item for item in hardware}
        if any((item["physical_state"], item["evidence_state"], item["functional_state"],
                item["integration_state"]) != _EXPECTED_HARDWARE_STATES[item_id]
               or item["label"] != _EXPECTED_HARDWARE_LABELS[item_id]
               or set(item["restrictions"]) != _EXPECTED_HARDWARE_RESTRICTIONS[item_id]
               or len(item["restrictions"]) != len(_EXPECTED_HARDWARE_RESTRICTIONS[item_id])
               for item_id, item in by_id.items()):
            return fallback
        return {**value, "evidence_status": "VALIDATED_REPOSITORY_EVIDENCE"}
    except (OSError, ValueError, TypeError, AttributeError):
        return fallback


def clear_project_evidence_cache() -> None:
    """Compatibility hook: physical/project evidence is deliberately never cached."""
    return None


def project_evidence() -> dict[str, Any]:
    try:
        if PROJECT_EVIDENCE.is_symlink():
            return _invalid_project_evidence()
        metadata = PROJECT_EVIDENCE.stat()
        if metadata.st_size > 32768:
            return _invalid_project_evidence()
        return deepcopy(_read_project_evidence(
            str(PROJECT_EVIDENCE), metadata.st_mtime_ns, metadata.st_size))
    except OSError:
        return _invalid_project_evidence()


def esp32_development_status() -> dict[str, Any]:
    evidence = project_evidence()["software"]
    status = {
        "toolchain": evidence["esp32_toolchain"],
        "generic_cross_build": evidence["esp32_generic_cross_build"],
        "generic_target": evidence["esp32_target"], "generic_profile": evidence["esp32_profile"],
        "verified_board": "BLOCKED_HW_002_NOT_ARRIVED",
        "physical_target": evidence["esp32_physical_target"], "flash": evidence["esp32_flash"],
        "local_build_record": "NOT_PRESENT",
    }
    try:
        if ESP32_BUILD_STATUS.is_symlink() or ESP32_BUILD_STATUS.stat().st_size > 4096:
            raise ValueError("invalid build status file")
        report = json.loads(ESP32_BUILD_STATUS.read_text(encoding="utf-8-sig"))
        if (report.get("schema") == "zie.esp32-generic-build.v1"
                and report.get("target") == "esp32s3"
                and report.get("profile") == "GENERIC_UNVERIFIED_ESP32S3"
                and report.get("result") == "PASS"
                and report.get("flash") == "NOT_AUTHORIZED"
                and re.fullmatch(r"ESP-IDF v\d+\.\d+\.\d+", str(report.get("idf_version")))):
            status["local_build_record"] = "VALIDATED_PASS"
            status["idf_version"] = report.get("idf_version")
        else:
            status["local_build_record"] = "INVALID_IGNORED"
    except (OSError, ValueError, TypeError, json.JSONDecodeError):
        if ESP32_BUILD_STATUS.exists():
            status["local_build_record"] = "INVALID_IGNORED"
    return status


@lru_cache(maxsize=1)
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


class WorkspacePreview:
    """Process-local preview state with no hardware or HostRuntime authority."""
    def __init__(self, simulation: bool):
        self.simulation = simulation
        self.expression, self.face_pack = "neutral", "builtin-preview"
        self.brightness, self.animation_speed, self.rgb_brightness = 50, 50, 35
        self.rgb_pattern, self.test_pattern = "calm", False
        self.active_actuator: str | None = None
        self.actuator_positions = {slot: 0.0 for slot in ACTUATORS}
        self.drive = {"direction": "stop", "speed_limit": 25, "deadman": False,
                      "request_status": "SIMULATED" if simulation else "REJECTED"}
        self.drive_deadline = 0.0

    def presentation(self) -> dict[str, Any]:
        return {"state": "PREVIEW", "delivery": "SIMULATED" if self.simulation else "NOT_DELIVERED",
                "expression": self.expression, "face_pack": self.face_pack,
                "brightness": self.brightness, "animation_speed": self.animation_speed,
                "rgb_brightness": self.rgb_brightness, "rgb_pattern": self.rgb_pattern,
                "display_test_pattern": self.test_pattern, "display_health": "UNAVAILABLE",
                "rgb_fail_state": "fail-dark"}

    def set_presentation(self, values: dict[str, Any]) -> dict[str, Any]:
        if values["expression"] not in EXPRESSIONS:
            raise ValueError("unsupported semantic expression")
        for key in ("brightness", "animation_speed", "rgb_brightness"):
            if not 0 <= values[key] <= 100:
                raise ValueError(f"{key} is outside 0..100")
        for key in ("expression", "face_pack", "brightness", "animation_speed",
                    "rgb_brightness", "rgb_pattern", "display_test_pattern"):
            setattr(self, "test_pattern" if key == "display_test_pattern" else key, values[key])
        return self.presentation()

    def commission(self, slot: str, operation: str, speed: int) -> dict[str, Any]:
        if slot not in ACTUATORS or operation not in {"enable", "jog_plus", "jog_minus", "stop"}:
            raise ValueError("invalid semantic commissioning request")
        if not 1 <= speed <= 25:
            raise ValueError("test speed is outside safe simulation bound 1..25")
        if not self.simulation:
            return {"slot": slot, "operation": operation, "state": "REJECTED",
                    "delivery": "NOT_DELIVERED", "physically_confirmed": False}
        if operation == "enable":
            if self.active_actuator not in {None, slot}:
                raise RuntimeError("another commissioning actuator is active")
            self.active_actuator = slot
        elif operation in {"jog_plus", "jog_minus"}:
            if self.active_actuator != slot:
                raise RuntimeError("actuator must be explicitly enabled")
            delta = 1.0 if operation == "jog_plus" else -1.0
            self.actuator_positions[slot] = max(-10.0, min(10.0, self.actuator_positions[slot] + delta))
        elif self.active_actuator == slot:
            self.active_actuator = None
        elif self.active_actuator is not None:
            raise RuntimeError(f"stop rejected; active commissioning actuator is {self.active_actuator}")
        return {"slot": slot, "operation": operation, "state": "SIMULATED", "delivery": "SIMULATED",
                "requested_position": self.actuator_positions[slot], "speed": speed,
                "active_slot": self.active_actuator, "physically_confirmed": False}

    def drive_request(self, direction: str, speed_limit: int, deadman: bool) -> dict[str, Any]:
        if direction not in DRIVE_DIRECTIONS or not 0 <= speed_limit <= 40:
            raise ValueError("invalid bounded drive request")
        accepted = self.simulation and (direction == "stop" or deadman)
        self.drive = {"direction": direction if accepted else "stop", "speed_limit": speed_limit,
                      "deadman": deadman, "request_status": "SIMULATED" if accepted else "REJECTED"}
        self.drive_deadline = time.monotonic() + 0.5 if accepted and direction != "stop" else 0.0
        return {**self.drive, "phase": "ACCEPTED" if accepted else "REJECTED",
                "delivery": "SIMULATED" if accepted else "NOT_DELIVERED", "physically_confirmed": False}

    def drive_status(self) -> dict[str, Any]:
        if self.drive["direction"] != "stop" and time.monotonic() >= self.drive_deadline:
            self.drive = {**self.drive, "direction": "stop", "deadman": False,
                          "request_status": "DEADMAN_EXPIRED"}
            self.drive_deadline = 0.0
        return dict(self.drive)


def cockpit_status(runtime: dict[str, Any], providers: dict[str, Any], stt: str, tts: str,
                   preview: WorkspacePreview | None = None) -> dict[str, Any]:
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
    preview = preview or WorkspacePreview(simulation)
    presence = unverified_presence_status()
    media = media_capability_status()
    evidence = project_evidence()
    return {
        "version": APP_VERSION, "stage": APP_STAGE,
        "development_readiness": evidence["software"],
        "bench_evidence": evidence["hardware"],
        "evidence_status": evidence["evidence_status"],
        "state_vocabulary": APP_STATES,
        "workspaces": WORKSPACES,
        "target": {
            "mode": mode, "runtime": runtime, "physical_readiness": "NOT_VERIFIED",
            "execution_confirmation": "PHYSICALLY_CONFIRMED" if real_confirmed else "NOT_VERIFIED",
        },
        "controllers": [
            {"id": "esp32", "role": "multimedia/presence/display/camera/audio", "state": hardware_state,
             "identity": None, "firmware": None, "session_id": None, "heartbeat_age": None,
             "uptime": None, "reset_reason": None, "faults": [], "commissioning": "NOT_VERIFIED"},
            {"id": "stm32", "role": "protected motion/safety/sensors/limits/faults", "state": hardware_state,
             "identity": None, "firmware": None, "session_id": None, "heartbeat_age": None,
             "uptime": None, "reset_reason": None, "faults": [], "commissioning": "NOT_VERIFIED"},
        ],
        "camera": {"state": "TEST_SOURCE" if simulation else "UNAVAILABLE",
                   "mode": "TEST_SOURCE" if simulation else "UNAVAILABLE", "allowed_modes": CAMERA_MODES,
                   "source": "generated-static-frame" if simulation else None,
                   "resolution": "640x360" if simulation else None, "fps": 0 if simulation else None,
                   "latency_ms": None, "stream": False,
                   "snapshot": "AVAILABLE" if simulation else "UNAVAILABLE",
                   "preview_url": "/hud/test-frame.svg" if simulation else None,
                   "overlays": "OFF", "connection_error": None if simulation else "physical camera not connected"},
        "presentation": preview.presentation(),
        "presence": presence,
        "media": media,
        "face_engine": {
            "mode": "OFFLINE_AUTONOMOUS",
            "mode_source": "HOST_PRESENCE_LINK",
            "renderer": "HOST_PREVIEW_MODEL_ONLY",
            "pack": "zie-core-procedural",
            "state": "IDLE",
            "emotion": "NEUTRAL",
            "variant": "idle-core",
            "physical_delivery": "NOT_DELIVERED",
            "may_originate_motion": False,
        },
        "actuators": [
            {"slot": slot, "state": "SIMULATED" if simulation else "UNAVAILABLE", "identity": None,
             "position": preview.actuator_positions[slot] if simulation else None,
             "requested_position": preview.actuator_positions[slot] if simulation else None,
             "speed": None, "range": [-10.0, 10.0] if simulation else None, "current": None,
             "temperature": None, "fault": None, "commissioning": "NOT_VERIFIED",
             "controls": "semantic-bounded-simulation", "active": preview.active_actuator == slot} for slot in ACTUATORS
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
        "voice": {"stt": stt, "tts": tts, "providers": providers,
                  "wake": "UNVERIFIED", "local_safety_voice": "MODEL_ONLY",
                  "critical_grammar": ["STOP", "FREEZE", "EMERGENCY_STOP"]},
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
            "motion_blocked_reasons": (["real target unavailable"] if not simulation else []) + [
                "physical commissioning gates incomplete", "cliff sensors uncommissioned",
                "battery protection unverified", "controller link unavailable",
            ],
        },
        "drive": preview.drive_status(),
        "firmware": {
            "esp32": {"detected": False, "firmware": None, "approved_target": None,
                      **esp32_development_status()},
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
    kind: str = "bounded_process"

    def public(self) -> dict[str, Any]:
        value = asdict(self)
        value.pop("command")
        return value


def _contained_log_path(log_path: Path) -> Path | None:
    resolved = (REPOSITORY_ROOT / log_path).resolve() if not log_path.is_absolute() else log_path.resolve()
    runtime_root = (REPOSITORY_ROOT / "runtime").resolve()
    return resolved if resolved == runtime_root or runtime_root in resolved.parents else None


def developer_actions(log_path: Path | None = None, hermes_command: str | None = None) -> dict[str, DeveloperAction]:
    cadquery_available = importlib.util.find_spec("cadquery") is not None
    terminal = shutil.which("wt.exe") or shutil.which("wt")
    explorer = shutil.which("explorer.exe") or shutil.which("explorer")
    codex = shutil.which("codex.exe") or shutil.which("codex")
    hermes = (shutil.which("hermes.exe") or shutil.which("hermes")) if hermes_command == "hermes" else None
    safe_log = _contained_log_path(log_path or Path("runtime/zie.log.jsonl"))
    return {
        "app_tests": DeveloperAction("app_tests", "Run full App pytest", True,
                                     (sys.executable, "-m", "pytest", "tests/app", "-q"), 180),
        "fallback_tests": DeveloperAction("fallback_tests", "Run fallback tests", True,
                                          (sys.executable, "scripts/run_tests_fallback.py"), 180),
        "compileall": DeveloperAction("compileall", "Compile Python App", True,
                                      (sys.executable, "-m", "compileall", "-q", "app", "tests/app"), 120),
        "cad_checks": DeveloperAction("cad_checks", "Run current CAD checks", cadquery_available,
                                      (sys.executable, "-B", "src/validate_design.py") if cadquery_available else None, 180),
        "open_repository": DeveloperAction("open_repository", "Open repository folder", bool(explorer), (explorer, str(REPOSITORY_ROOT)) if explorer else None, 10, "detached_launch"),
        "open_terminal": DeveloperAction("open_terminal", "Open Windows Terminal at repository", bool(terminal), (terminal, "-d", str(REPOSITORY_ROOT)) if terminal else None, 10, "detached_launch"),
        "launch_codex": DeveloperAction("launch_codex", "Launch Codex", bool(terminal and codex), (terminal, "-d", str(REPOSITORY_ROOT), codex, "--cd", str(REPOSITORY_ROOT), "--sandbox", "workspace-write", "--ask-for-approval", "never", "--search") if terminal and codex else None, 10, "detached_launch"),
        "launch_hermes": DeveloperAction("launch_hermes", "Launch Hermes", bool(terminal and hermes), (terminal, "-d", str(REPOSITORY_ROOT), hermes) if terminal and hermes else None, 10, "detached_launch"),
        "open_logs": DeveloperAction("open_logs", "Open App logs", bool(explorer and safe_log), (explorer, "/select,", str(safe_log)) if explorer and safe_log else None, 10, "detached_launch"),
    }


async def run_developer_action(action_id: str, log_path: Path | None = None, hermes_command: str | None = None) -> dict[str, Any]:
    action = developer_actions(log_path, hermes_command).get(action_id)
    if action is None:
        raise KeyError("unknown developer action")
    if not action.available or action.command is None:
        raise RuntimeError("developer action is unavailable")
    cwd = REPOSITORY_ROOT / "mechanical" / "cad" / "current" if action_id == "cad_checks" else REPOSITORY_ROOT
    if action.kind == "detached_launch":
        try:
            subprocess.Popen(action.command, cwd=cwd, shell=False, stdin=subprocess.DEVNULL,
                             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                             creationflags=getattr(subprocess, "DETACHED_PROCESS", 0) |
                             getattr(subprocess, "CREATE_NEW_PROCESS_GROUP", 0))
        except OSError as exc:
            raise RuntimeError("fixed local application launch is unavailable") from exc
        return {"action_id": action_id, "state": "ACCEPTED", "exit_code": None,
                "output": "fixed local application launch requested", "robot_authority": "NONE"}
    process = await asyncio.create_subprocess_exec(
        *action.command, cwd=cwd, stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.STDOUT,
    )
    try:
        output, _ = await asyncio.wait_for(process.communicate(), timeout=action.timeout)
    except asyncio.TimeoutError:
        process.kill()
        await process.communicate()
        return {"action_id": action_id, "state": "TIMEOUT", "exit_code": None, "output": "timed out", "robot_authority": "NONE"}
    decoded = redact_text(output.decode("utf-8", errors="replace")[-20000:])
    return {
        "action_id": action_id,
        "state": "PASSED" if process.returncode == 0 else "FAILED",
        "exit_code": process.returncode,
        "output": decoded,
        "robot_authority": "NONE",
    }
