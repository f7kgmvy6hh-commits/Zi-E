from __future__ import annotations

from dataclasses import asdict, dataclass
from difflib import SequenceMatcher
import re
from typing import Any


SOURCE = "docs/HARDWARE_SOURCE_OF_TRUTH_RECONCILIATION.md"
PHASE2B2_STATE = "WAITING_FOR_VERIFIED_INPUTS"


@dataclass(frozen=True)
class Requirement:
    requirement_id: str
    subsystem: str
    requirement: str
    candidate_reference: str
    manufacturer: str
    model: str
    owner_domain: str
    logical_slot: str
    optionality: str
    safety_priority: str
    gates: tuple[int, ...]


REQUIREMENTS = (
    Requirement("compute.esp32", "COMPUTE", "ESP32-S3 multimedia controller", "ESP32-S3-WROOM-1-N16R8 candidate", "Espressif", "ESP32-S3-WROOM-1-N16R8", "ESP32", "controller.multimedia", "REQUIRED", "P0", (5, 7, 8)),
    Requirement("compute.stm32", "COMPUTE", "STM32 protected controller", "STM32G0B1RET6 candidate/custom carrier reservation", "STMicroelectronics", "STM32G0B1RET6", "PROTECTED_SAFETY", "controller.safety", "REQUIRED", "P0", (6, 7, 8)),
    Requirement("ui.display", "DISPLAY_UI", "face display/touch", "3.5-inch ILI9488 display candidate", "", "ILI9488", "ESP32", "display.face", "REQUIRED", "P1", (13,)),
    Requirement("ui.belly", "DISPLAY_UI", "belly RGB matrix", "Adafruit IS31FL3741 13x9 prototype matrix", "Adafruit", "IS31FL3741", "ESP32", "rgb.belly", "REQUIRED", "P3", (13,)),
    Requirement("vision.camera", "CAMERA_VISION", "head camera", "OV5640 5 MP autofocus camera candidate", "", "OV5640", "ESP32", "camera.head", "REQUIRED", "P1", (13,)),
    Requirement("range.head", "RANGE_PROXIMITY", "head rangefinder", "VL53L1X head rangefinder", "STMicroelectronics", "VL53L1X", "ESP32", "range.head", "REQUIRED", "P1", (9, 13)),
    Requirement("safety.cliff", "CLIFF_BUMPER_SAFETY", "downward cliff sensors", "VL53L4CD downward cliff sensors", "STMicroelectronics", "VL53L4CD", "PROTECTED_SAFETY", "cliff.downward.*", "REQUIRED", "P0", (9, 12, 14, 15, 16)),
    Requirement("safety.bumper", "CLIFF_BUMPER_SAFETY", "bumper switches", "D2F-L-class bumper switches", "Omron", "D2F-L", "PROTECTED_SAFETY", "bumper.contact.*", "REQUIRED", "P0", (9, 12, 15, 16)),
    Requirement("sense.proximity", "RANGE_PROXIMITY", "horizontal proximity sensors", "VL53L4CD horizontal proximity conditional", "STMicroelectronics", "VL53L4CD", "STM32", "proximity.horizontal.*", "CONDITIONAL", "P2", (9,)),
    Requirement("sense.imu", "IMU", "body IMU", "LSM6DSOX IMU candidate", "STMicroelectronics", "LSM6DSOX", "STM32", "imu.body", "REQUIRED", "P2", (9,)),
    Requirement("drive.motor", "DRIVE_MOTORS", "encoded drive motors", "25SG-370CA-78-EN-class motors", "", "25SG-370CA-78-EN", "PROTECTED_SAFETY", "drive.left/right", "REQUIRED", "P0", (10, 11, 14, 15, 16)),
    Requirement("drive.driver", "MOTOR_DRIVERS", "protected motor drivers", "DRV8876 protected H-bridge class", "Texas Instruments", "DRV8876", "PROTECTED_SAFETY", "drive.power.left/right", "REQUIRED", "P0", (10, 11, 12, 14)),
    Requirement("drive.wheel", "WHEELS_CASTER", "drive wheels", "64 mm-class wheel/tire", "", "64mm wheel", "PASSIVE", "drive.wheel.*", "REQUIRED", "P1", (14, 15, 16)),
    Requirement("support.caster", "WHEELS_CASTER", "passive caster", "POM ball-caster cartridge candidate", "", "POM ball caster", "PASSIVE", "support.caster", "REQUIRED", "P1", (14, 15, 16)),
    Requirement("motion.shoulder", "ARM_ACTUATORS", "shoulder servos", "Feetech STS3215-C018 candidate", "Feetech", "STS3215-C018", "STM32", "arm.*.shoulder.*", "REQUIRED", "P1", (10, 11, 15)),
    Requirement("motion.elbow", "ARM_ACTUATORS", "elbow servos", "Feetech HL-2915-C001 candidate", "Feetech", "HL-2915-C001", "STM32", "arm.*.elbow", "REQUIRED", "P1", (10, 11, 15)),
    Requirement("motion.small", "ARM_ACTUATORS", "wrist/head/gripper servos", "Waveshare SC09-class candidates", "Waveshare", "SC09", "STM32", "arm/head/gripper.*", "REQUIRED", "P1", (10, 11, 15)),
    Requirement("motion.linear", "LIFT_TELESCOPE", "lift/telescope mechanisms", "N20 encoder motor + T5x2 lead candidate", "", "N20 T5x2", "STM32", "arm.*.telescope/head.lift", "REQUIRED", "P1", (10, 11, 15)),
    Requirement("gripper.mechanism", "GRIPPERS", "adaptive grippers", "Adaptive tendon gripper mechanism", "", "adaptive tendon gripper", "STM32", "gripper.left/right", "REQUIRED", "P1", (10, 11, 15)),
    Requirement("power.battery", "BATTERY", "battery cells/pack architecture", "Molicel M35A-class 18650 3S2P", "Molicel", "M35A", "PROTECTED_SAFETY", "battery.main", "REQUIRED", "P0", (1, 3, 4)),
    Requirement("power.protection", "PROTECTION_BMS", "pack protection", "BQ7791500-class protection candidate", "Texas Instruments", "BQ7791500", "PROTECTED_SAFETY", "power.pack_protection", "REQUIRED", "P0", (1, 3, 4, 12)),
    Requirement("power.charger", "CHARGING", "external charger", "Certified external 12.6 V CC/CV charger", "", "12.6V CC/CV charger", "PASSIVE", "power.charger_external", "REQUIRED", "P0", (1, 3, 4)),
    Requirement("power.logic", "POWER_REGULATION", "logic rail regulators", "AP62300-class general buck regulators", "Diodes Incorporated", "AP62300", "PROTECTED_SAFETY", "power.logic_rail.*", "REQUIRED", "P0", (3, 4)),
    Requirement("power.safety", "POWER_REGULATION", "safety 3.3 V regulator", "TPS62162-class safety supply", "Texas Instruments", "TPS62162", "PROTECTED_SAFETY", "power.safety_3v3", "REQUIRED", "P0", (3, 4, 6)),
    Requirement("power.branch", "POWER_REGULATION", "actuator branch protection", "TPS25947/eFuse-class protection", "Texas Instruments", "TPS25947", "PROTECTED_SAFETY", "power.actuator_branch.*", "REQUIRED", "P0", (3, 4, 10, 12)),
    Requirement("link.can", "CAN_CONTROLLER_LINK", "CAN transceiver/link hardware", "ESP32–STM32 Classical CAN/TWAI link", "", "Classical CAN transceiver", "PROTECTED_SAFETY", "link.controller", "REQUIRED", "P0", (7, 8, 12)),
    Requirement("audio.input", "AUDIO", "microphones", "DMM-4026 I2S-class microphones", "", "DMM-4026", "ESP32", "audio.input.*", "REQUIRED", "P3", (13,)),
    Requirement("audio.output", "AUDIO", "speaker", "PUI AS04004PO-R-class speaker", "PUI", "AS04004PO-R", "ESP32", "audio.output", "REQUIRED", "P3", (13,)),
    Requirement("audio.amplifier", "AUDIO", "audio amplifier", "MAX98357A-class I2S amplifier", "Analog Devices", "MAX98357A", "ESP32", "audio.amplifier", "REQUIRED", "P3", (13,)),
    Requirement("structure.hardware", "STRUCTURAL_MECHANICAL", "stops/brackets/fasteners", "Head/arm/wheel structural hardware", "", "structural hardware", "PASSIVE", "physical.assembly", "REQUIRED", "P1", (1, 2, 15)),
)

DOCUMENTED_PRIMARY_CANDIDATE_COUNT = 34

FIRST_BENCH_ITEM_PLANS = {
    "HW-001": ("inventory spare ESP32-S3 development board", "BLOCKED_PENDING_EXPLICIT_PHYSICAL_IDENTITY_LINK",
        ["Explicitly link the photographed HW-001 unit to its physical PCB/module markings",
         "Do not infer that the separately tested HW678 board is HW-001 from matching family specifications"],
        "No powered action is authorized; SPARE-HW678 evidence is separate until the inventory identity is proven."),
    "HW-002": ("ordered ESP32-S3 camera board", "BLOCKED_UNTIL_ARRIVAL",
        ["front/back photo", "board/revision and ESP32 module label close-up", "OV5640 sensor marking close-up",
         "camera FPC/connector orientation close-up", "both USB connectors/chips close-up",
         "exact purchase/order page", "board-vendor schematic and pinout"],
        "After arrival identity review: USB-only board boot, then camera example using the reviewed board pin map."),
    "HW-003": ("ILI9488 display/touch module", "DO_NOT_CONNECT_YET",
        ["front/back photo", "PCB revision and display/touch-controller label close-up", "every connector/pin label close-up",
         "VCC and logic-level documentation", "touch-controller and microSD schematic/datasheet reference"],
        "After voltage/interface review: current-limited display-only test; touch remains separate until its controller is identified."),
    "HW-004": ("PCM5102/PCM5102A DAC module", "DO_NOT_CONNECT_YET",
        ["front/back photo", "IC and regulator/jumper label close-up", "all power/I2S/output pin labels",
         "module VIN/logic documentation", "exact module schematic or purchase page"],
        "After voltage/interface review: isolated DAC-only I2S test into a non-powered measurement/load path."),
    "HW-005": ("PAM8403 amplifier module", "DO_NOT_CONNECT_YET",
        ["front/back photo", "IC and terminal/pin label close-up", "module supply documentation",
         "multimeter continuity check confirming speaker negatives are not ground"],
        "After evidence review: current-limited amplifier-only test into one known speaker; never ground or join bridge outputs."),
    "HW-006": ("MAX9814 microphone module", "INDEPENDENT_BENCH_PASS_INTEGRATION_NOT_COMMISSIONED",
        ["Preserve the 1215-1219 mV bias and 393-767 mV peak-to-peak response record",
         "Review production placement, ADC assignment, and electrical integration separately"],
        "No further powered action is authorized here; temporary GPIO4 is not a production assignment."),
    "HW-007": ("VL53L0X range module", "QUARANTINED_RANGING_NOT_VERIFIED",
        ["Preserve identity evidence: address 0x29 and register 0xC0 = 0xEE",
         "Rework or replace before any new ranging attempt", "Record an explicit KEEP/REPLACE decision"],
        "Do not power in this pass; temporary GPIO5/GPIO6 are not production assignments."),
    "HW-008": ("WS2812-family RGB breakouts", "INDEPENDENT_BENCH_PASS_FINAL_5V_NOT_VERIFIED",
        ["Preserve 5V/DI/GND input and 5V/DO/GND output orientation evidence",
         "Preserve the red/green/blue/off sequence and approximately 330 ohm series-resistor measurement",
         "Verify a suitable final 5 V source and integration separately",
         "record an explicit KEEP/REPLACE decision versus the belly matrix"],
        "3.3 V bench operation passed via temporary GPIO7; GPIO7 is not production, final 5 V is unverified, and HW678 5Vin is not a verified source."),
    "HW-009": ("8 ohm speakers", "UNPOWERED_IDENTIFICATION_ONLY",
        ["front/back and marking photo", "diameter/depth/mounting measurements", "multimeter DC-resistance measurement per speaker"],
        "After amplifier evidence review: test one speaker at bounded level; speaker evidence alone does not authorize amplifier power."),
    "HW-010": ("5 V / 3 A seller-described power board", "DO_NOT_POWER_OR_CONNECT",
        ["front/back high-resolution photo", "all IC/transformer/regulator markings", "input/output terminal label close-up",
         "exact purchase/order page", "manufacturer schematic/datasheet", "documented input voltage/range and frequency/type",
         "documented output voltage/tolerance", "polarity/line-neutral/earth identification",
         "isolation or non-isolation evidence", "continuous/peak current and thermal conditions",
         "short-circuit/over-current/over-voltage/thermal protection evidence", "required upstream fuse/eFuse specification",
         "connector pitch/type/rating", "unpowered continuity/isolation check plan",
         "current-limited unloaded-output measurement plan with operator disconnect"],
        "No powered test until board identity and safe input method are reviewed; never attach it to robot loads for initial characterization."),
    "HW-016": ("USB-A to USB-C cable", "UNPOWERED_OR_HOST_ONLY_IDENTIFICATION",
        ["connector close-up", "data-capability test on a known non-robot USB device", "continuity/shield result if safely measurable"],
        "May be checked on a known low-risk USB device; do not use it as board evidence or assume data capability."),
    "HW-017": ("digital multimeter", "PRESENT_ACTIVE_BENCH_TOOL",
        ["Record exact model/range/fuse/category evidence when relevant to a higher-risk measurement",
         "continue to reject unverified CAT claims"],
        "Low-voltage bench use is observed; this status does not authorize mains work."),
    "HW-022": ("latching pushbuttons", "UNPOWERED_CONTINUITY_AFTER_METER_REVIEW",
        ["pin and body close-up", "continuity matrix in both switch states", "contact-rating datasheet/order page"],
        "Unpowered contact identification only; not a verified power disconnect."),
    "HW-023": ("slide switches", "UNPOWERED_CONTINUITY_AFTER_METER_REVIEW",
        ["pin and body marking close-up", "continuity matrix in both positions", "contact-rating datasheet/order page"],
        "Unpowered common/contact identification only; not a verified power disconnect."),
}


def _first_bench_readiness(items: list[dict[str, Any]], records: list[dict[str, Any]]) -> dict[str, Any]:
    by_id = {item["inventory_id"]: item for item in items}
    plans = []
    for inventory_id, (name, status, evidence, conditional_test) in FIRST_BENCH_ITEM_PLANS.items():
        item = by_id.get(inventory_id)
        if not item:
            continue
        plans.append({"priority":"P0" if inventory_id in {"HW-002", "HW-010", "HW-017"} else "P1",
            "inventory_id":inventory_id, "item":name, "physical_status":item["physical_status"],
            "bench_status":status, "evidence_to_provide":evidence,
            "conditional_independent_test":conditional_test, "powered_test_authorized":False})
    received = [item for item in items if item.get("physical_status") == "RECEIVED"]
    conflicts = [{"inventory_id":record["inventory_id"], "reason":record["purchased"].get("mismatch_reason")
                  or record["purchased"].get("conflict_reason")}
                 for record in records if record["quarantined"]]
    return {
        "state":"EVIDENCE_COLLECTION_ONLY",
        "first_power_authorized":False,
        "powered_subsystem_test_authorized":False,
        "can_do_now":[
            "Photograph received articles, labels, both PCB sides, connectors, pin legends, and packaging/order references.",
            "Measure unpowered mechanical dimensions and connector pitch/orientation.",
            "Use the present multimeter only within its evidenced low-voltage bench scope; do not infer mains/CAT capability.",
            "Check HW-016 data capability using a known low-risk non-robot USB device.",
        ],
        "do_not_connect":[
            "Do not energize HW-010 or connect it to any robot/subsystem load.",
            "Do not connect received active peripherals to ESP32 GPIO or power until each module's VIN, logic levels, pin labels, and interface are reviewed.",
            "Do not use workbook seller GPIO/wiring/firmware sheets as physical pin evidence.",
            "Do not build a final harness, common-ground tree, or production pin assignment.",
            "Do not connect motion, battery/BMS, STM32, or CAN paths; required hardware/evidence is absent.",
        ],
        "power":{"state":"BLOCKED_UNVERIFIED_POWER_HARDWARE", "inventory_ids":[x for x in ("HW-010",) if x in by_id],
            "other_received_regulators":[], "candidate_only_not_owned":["HW-033 XL4015E"]},
        "camera":{"state":"BLOCKED_UNTIL_ARRIVAL_AND_REVIEW", "inventory_ids":[x for x in ("HW-002",) if x in by_id],
            "seller_maps":"PROVISIONAL_ONLY", "production_pin_freeze":False},
        "wiring":{"state":"PROVISIONAL_ENGINEERING_PLAN_ONLY", "verified_physical_pin_mappings":[],
            "final_harness_freeze":False, "production_pin_freeze":False,
            "missing":["connector identity/pitch/mating/orientation", "module VIN and logic levels",
                "polarity and return paths", "common-ground/current budget", "USB data capability"]},
        "stm32_motion_can":{"state":"BLOCKED_MISSING_REQUIRED_HARDWARE_AND_EVIDENCE",
            "phase2b2":PHASE2B2_STATE, "commissioning_passes":0},
        "received_inventory_count":len(received), "item_plans":plans, "conflicts":conflicts,
        "robot_authority":"NONE",
    }


def normalized(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "", value.casefold())


def _suggestion(item: dict[str, Any]) -> tuple[str, Requirement | None]:
    maker, model = normalized(item.get("manufacturer", "")), normalized(item.get("model_exact_variant", ""))
    if not model:
        return "NO_CANDIDATE", None
    exact = [req for req in REQUIREMENTS if normalized(req.model) == model and
             (not normalized(req.manufacturer) or (maker and normalized(req.manufacturer) == maker))]
    if len(exact) == 1:
        return "MATCH_REVIEW_REQUIRED", exact[0]
    if len(exact) > 1:
        return "CONFLICT", None
    scored = sorted(((SequenceMatcher(None, model, normalized(req.model)).ratio(), req) for req in REQUIREMENTS),
                    key=lambda pair: (-pair[0], pair[1].requirement_id))
    return ("POSSIBLE_MATCH", scored[0][1]) if scored and scored[0][0] >= 0.72 else ("NO_CANDIDATE", None)


def _identity(item: dict[str, Any]) -> str:
    if item.get("verified"): return "VERIFIED"
    if item.get("review_state") == "REVIEWED" and item.get("manufacturer") and item.get("model_exact_variant"): return "REVIEWED"
    if item.get("manufacturer") and item.get("model_exact_variant"): return "IDENTIFIED"
    if item.get("manufacturer") or item.get("model_exact_variant"): return "PARTIAL"
    return "UNKNOWN"


def _blockers(item: dict[str, Any], match: str) -> list[str]:
    values = []
    if _identity(item) in {"UNKNOWN", "PARTIAL"}: values.append("BLOCKED_ON_IDENTITY")
    if not item.get("evidence_source"): values.append("BLOCKED_ON_DATASHEET")
    if not item.get("dimensions_measured") and item.get("cad_status") != "NOT_APPLICABLE": values.append("BLOCKED_ON_MEASUREMENT")
    if match in {"MISMATCH", "CONFLICT"} or item.get("conflict_reason"): values.append("DESIGN_CONFLICT")
    if item.get("safety_criticality", "").lower() in {"critical", "high"} and item.get("review_state") != "REVIEWED": values.append("SAFETY_REVIEW_REQUIRED")
    if not item.get("zie_logical_slot"): values.append("PROFILE_MAPPING_REQUIRED")
    if not item.get("driver_extension"): values.append("DRIVER_NOT_STARTED")
    if not (item.get("voltage_documented") and item.get("current_documented") and item.get("interface_documented")): values.append("ELECTRICAL_EVIDENCE_REQUIRED")
    return values


def _next_action(item: dict[str, Any], match: str, blockers: list[str]) -> str:
    ordered = ((not item.get("model_exact_variant"), "Photograph and transcribe the exact model label."),
        (not item.get("manufacturer"), "Record the manufacturer from the physical label or order evidence."),
        (item.get("physical_status") == "UNKNOWN", "Confirm whether the item is received, ordered, or not bought."),
        (not item.get("photo_reference"), "Add a reference to a photo of the exact label and connectors."),
        (not item.get("purchase_link_reference"), "Add the purchase or order reference."),
        (not item.get("evidence_source"), "Find and reference the manufacturer datasheet."),
        (match in {"POSSIBLE_MATCH", "MATCH_REVIEW_REQUIRED"}, "Review the design-candidate match."),
        (not item.get("dimensions_measured") and item.get("cad_status") != "NOT_APPLICABLE", "Measure the CAD-relevant dimensions and attach evidence."),
        (not item.get("weight_measured") and item.get("cad_status") != "NOT_APPLICABLE", "Measure mass and attach evidence."),
        (not item.get("connector"), "Identify the connector and orientation from evidence."),
        (not item.get("zie_logical_slot"), "Assign the reviewed semantic logical slot."),
        (item.get("decision_keep_replace_undecided") == "UNDECIDED", "Record an explicit KEEP or REPLACE decision after review."),
        (not item.get("verified"), "Complete reviewed physical verification evidence."))
    return next((action for condition, action in ordered if condition), "Physical commissioning is still required.")


def build_reports(items: list[dict[str, Any]]) -> dict[str, Any]:
    records = []
    for source in items:
        item = dict(source); suggestion, candidate = _suggestion(item)
        reviewed_match = {"NONE":"NO_CANDIDATE"}.get(item.get("candidate_match", "NO_CANDIDATE"), item.get("candidate_match", "NO_CANDIDATE"))
        match = reviewed_match if reviewed_match not in {"NO_CANDIDATE", "POSSIBLE_MATCH"} else suggestion
        blockers = _blockers(item, match)
        quarantine = bool(item.get("conflict_reason")) or match in {"MISMATCH", "CONFLICT"} or item.get("review_state") == "CONFLICT"
        readiness = "BLOCKED" if quarantine else ("WAITING_FOR_EVIDENCE" if blockers else
                    ("COMMISSIONING_REQUIRED" if item.get("driver_state") == "IMPLEMENTED" else "READY_FOR_ENGINEERING"))
        records.append({"inventory_id":item["inventory_id"], "purchased":item, "identity_status":_identity(item),
            "candidate_match":match, "candidate_suggestion":candidate.candidate_reference if candidate else None,
            "candidate_reference":item.get("candidate_reference") or None,
            "quarantined":quarantine, "readiness":readiness, "blockers":blockers,
            "next_action":_next_action(item, match, blockers), "robot_authority":"NONE"})
    by_requirement: dict[str, list[dict[str, Any]]] = {req.requirement_id:[] for req in REQUIREMENTS}
    for record in records:
        for req in REQUIREMENTS:
            item = record["purchased"]
            reviewed_candidate = (item.get("review_state") == "REVIEWED" and
                                  item.get("candidate_match") == "MATCHED" and
                                  record["candidate_reference"] == req.candidate_reference)
            reviewed_slot = (item.get("review_state") == "REVIEWED" and
                             item.get("zie_logical_slot") == req.logical_slot)
            if reviewed_candidate or reviewed_slot:
                by_requirement[req.requirement_id].append(record)
    coverage, missing = [], []
    for req in REQUIREMENTS:
        matches = by_requirement[req.requirement_id]; purchased = bool(matches)
        status = ("OPTIONAL_CONDITIONAL" if req.optionality != "REQUIRED" and not purchased else
                  "MISSING_PURCHASE" if not purchased else
                  "ORDERED_NOT_RECEIVED" if all(x["purchased"]["physical_status"] == "ORDERED" for x in matches) else
                  "RECEIVED_IDENTITY_UNKNOWN" if any(x["purchased"]["physical_status"] == "RECEIVED" and x["identity_status"] in {"UNKNOWN","PARTIAL"} for x in matches) else
                  "CONFLICT" if any(x["quarantined"] for x in matches) else
                  "REPLACEMENT_REQUIRED" if any(x["purchased"]["decision_keep_replace_undecided"] == "REPLACE" for x in matches) else
                  "RECEIVED_REVIEW_REQUIRED" if not all(x["purchased"]["verified"] for x in matches) else "REVIEWED_HARDWARE")
        trustworthy = bool(matches) and all(x["purchased"].get("physical_status") == "RECEIVED" and
            x["purchased"].get("review_state") == "REVIEWED" and
            x["purchased"].get("evidence_state") in {"REVIEWED", "VERIFIED"} and not x["quarantined"] and
            x["purchased"].get("decision_keep_replace_undecided") == "KEEP" for x in matches)
        row = {**asdict(req), "gates":list(req.gates), "candidate_exists":True, "purchased_part_exists":purchased,
               "inventory_ids":[x["inventory_id"] for x in matches], "coverage_status":status,
               "exact_identity_known":bool(matches) and all(x["identity_status"] in {"IDENTIFIED","REVIEWED","VERIFIED"} for x in matches),
               "reviewed":bool(matches) and all(x["purchased"]["review_state"] == "REVIEWED" for x in matches),
               "decision":matches[0]["purchased"]["decision_keep_replace_undecided"] if len(matches)==1 else "UNDECIDED",
               "cad_ready":trustworthy and all(x["purchased"]["cad_status"] == "MATCHED" and x["purchased"]["dimensions_measured"] for x in matches),
               "electrical_ready":trustworthy and all(x["purchased"]["voltage_documented"] and x["purchased"]["current_documented"] and x["purchased"]["interface_documented"] and x["purchased"]["connector"] for x in matches),
               "driver_ready":trustworthy and all(x["identity_status"] in {"REVIEWED","VERIFIED"} and
                   x["purchased"].get("evidence_source") and x["purchased"].get("interface_documented") and
                   x["purchased"].get("controller_ownership") not in {"", "UNKNOWN"} and
                   x["purchased"].get("zie_logical_slot") for x in matches)}
        coverage.append(row)
        if status not in {"REVIEWED_HARDWARE", "OPTIONAL_CONDITIONAL"}: missing.append(row)
    first_bench = _first_bench_readiness(items, records)
    user_queue = []
    for plan in first_bench["item_plans"]:
        user_queue.append({"priority":plan["priority"], "inventory_id":plan["inventory_id"],
            "task":"Provide: " + "; ".join(plan["evidence_to_provide"]),
            "status":plan["bench_status"], "evidence_needed":plan["evidence_to_provide"],
            "purpose":"FIRST_BENCH_EVIDENCE_CLOSURE"})
    for req in missing:
        action = ("Enter the actual part/intent; provide receipt state, exact label and connector photos, "
                  "manufacturer/model, purchase reference, manufacturer datasheet, and relevant dimensions/mass."
                  if not req["purchased_part_exists"] else "Resolve the listed inventory evidence blockers.")
        user_queue.append({"priority":req["safety_priority"], "requirement_id":req["requirement_id"], "task":action,
                           "status":req["coverage_status"], "evidence_needed":["receipt_state", "exact_label_photo",
                           "connector_photo", "manufacturer_model", "purchase_reference", "datasheet",
                           "relevant_dimensions", "mass_if_cad_relevant"]})
    for record in records:
        user_queue.append({"priority":"P0" if "SAFETY_REVIEW_REQUIRED" in record["blockers"] else "P1",
                           "inventory_id":record["inventory_id"], "task":record["next_action"], "status":record["readiness"]})
    user_queue.sort(key=lambda x:(x["priority"], x.get("requirement_id", ""), x.get("inventory_id", "")))
    cad_relevant = [r for r in coverage if r["subsystem"] not in {"OTHER_UNMAPPED"}]
    cad_state = "CAD_EVIDENCE_INCOMPLETE" if not cad_relevant or not all(r["cad_ready"] for r in cad_relevant if r["optionality"]=="REQUIRED") else "READY_FOR_CAD_RECONCILIATION"
    electrical_relevant = [r for r in coverage if r["owner_domain"] != "PASSIVE"]
    electrical_state = "ELECTRICAL_EVIDENCE_INCOMPLETE" if not electrical_relevant or not all(r["electrical_ready"] for r in electrical_relevant if r["optionality"]=="REQUIRED") else "READY_FOR_ELECTRICAL_RECONCILIATION"
    driver_queue = [{"requirement_id":r["requirement_id"], "inventory_ids":r["inventory_ids"],
        "state":"READY_FOR_IMPLEMENTATION" if r["driver_ready"] else ("BLOCKED_ON_IDENTITY" if not r["exact_identity_known"] else "BLOCKED_ON_INTERFACE"),
        "controller_ownership":r["owner_domain"], "logical_slot":r["logical_slot"]} for r in coverage if r["owner_domain"] not in {"PASSIVE"}]
    phase_categories = ["CONTROLLER_IDENTITY", "TRANSCEIVER_IDENTITY", "TOPOLOGY", "CABLE_PHYSICAL_LINK",
        "CONTROLLER_CLOCK_FACTS", "EXPECTED_TRAFFIC", "BURST_ASSUMPTIONS", "QUEUE_ISR_DATA",
        "SAFE_STOP_REQUIREMENT", "PAYLOAD_FRAGMENTATION_REQUIREMENTS", "INTEGRITY_AUTH_SCOPE"]
    phase = {"state":PHASE2B2_STATE, "why_waiting":"No category has reviewed freeze evidence in the inventory model.",
             "categories":[{"category":x,"state":"UNKNOWN","value":None} for x in phase_categories]}
    gate_requirements = {1:("compute.esp32","compute.stm32","power.battery"),2:("structure.hardware",),3:("power.battery","power.protection","power.logic","power.safety"),4:("power.battery","power.protection","power.logic","power.safety"),5:("compute.esp32",),6:("compute.stm32",),7:("compute.esp32","compute.stm32","link.can"),8:("link.can",),9:("range.head","safety.cliff","safety.bumper","sense.imu"),10:("drive.motor","drive.driver","motion.shoulder"),11:("drive.motor","motion.shoulder","motion.elbow","motion.small","motion.linear"),12:("compute.stm32","safety.cliff","safety.bumper","drive.driver","link.can"),13:("ui.display","vision.camera","audio.input","audio.output"),14:("drive.motor","drive.driver","drive.wheel","support.caster"),15:("structure.hardware","drive.motor","motion.shoulder"),16:("structure.hardware","safety.cliff","safety.bumper","drive.motor")}
    coverage_map = {r["requirement_id"]:r for r in coverage}; gates=[]
    for number in range(1,17):
        unresolved=[rid for rid in gate_requirements[number] if coverage_map[rid]["coverage_status"] != "REVIEWED_HARDWARE"]
        gates.append({"gate":number,"required_hardware":list(gate_requirements[number]),"unresolved_hardware":unresolved,
                      "readiness":"BLOCKED" if unresolved else "READY_FOR_PHYSICAL_TEST","physical_result":"NOT_TESTED"})
    first_power_ids=("compute.esp32","compute.stm32","power.battery","power.protection","power.logic","power.safety","structure.hardware")
    first_unresolved=[rid for rid in first_power_ids if coverage_map[rid]["coverage_status"] != "REVIEWED_HARDWARE"]
    return {"source_of_truth":{"hierarchy":["DESIGN_CANDIDATE","PURCHASE_INTENT","PURCHASED_RECEIVED_INVENTORY","REVIEWED_ENGINEERING_IDENTITY","VERIFIED_HARDWARE"],"automatic_promotion":False},
        "candidate_catalog":{"source":SOURCE,"documented_primary_count":DOCUMENTED_PRIMARY_CANDIDATE_COUNT,
            "coverage_requirement_count":len(REQUIREMENTS),"purchased_inventory_seeded":False},
        "actual_active_inventory_count":len(items), "reconciliation_records":records,
        "hardware_reconciliation_summary":{"inventory_count":len(items),"quarantined":sum(r["quarantined"] for r in records),"undecided":sum(r["purchased"]["decision_keep_replace_undecided"]=="UNDECIDED" for r in records)},
        "required_hardware":coverage, "missing_hardware":missing, "user_input_required":user_queue,
        "cad_readiness":{"state":cad_state,"items":[r for r in coverage if not r["cad_ready"]]},
        "electrical_readiness":{"state":electrical_state,"items":[r for r in electrical_relevant if not r["electrical_ready"]]},
        "driver_implementation_queue":driver_queue,
        "esp32_readiness":{"state":"WAITING_FOR_EVIDENCE","items":[r for r in coverage if r["owner_domain"]=="ESP32"]},
        "stm32_readiness":{"state":"WAITING_FOR_EVIDENCE","items":[r for r in coverage if r["owner_domain"] in {"STM32","PROTECTED_SAFETY"}]},
        "phase2b2_inputs_required":phase, "commissioning_prerequisites":{"physical_passes":0,"gates":gates},
        "first_power_readiness":{"state":"PREREQUISITES_INCOMPLETE" if first_unresolved else "READY_FOR_PHYSICAL_TEST",
                                 "physical_power_authorized":False,"unresolved":first_unresolved},
        "first_bench_readiness":first_bench,
        "safety_critical_unresolved":[r for r in coverage if r["safety_priority"]=="P0" and r["coverage_status"]!="REVIEWED_HARDWARE"],
        "robot_authority":"NONE"}
