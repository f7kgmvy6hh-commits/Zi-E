from __future__ import annotations

import csv
from datetime import datetime, timezone
import io
import json
import os
from pathlib import Path
import re
import tempfile
import threading
from typing import Any


MAX_ROWS = 500
MAX_BYTES = 2 * 1024 * 1024
PHYSICAL_STATUSES = {"RECEIVED", "ORDERED", "NOT_BOUGHT", "UNKNOWN"}
DECISIONS = {"KEEP", "REPLACE", "UNDECIDED"}
OWNERS = {"ESP32", "STM32", "HOST", "PASSIVE", "PROTECTED_SAFETY", "UNKNOWN"}
EVIDENCE_STATES = {"UNKNOWN", "CLAIMED", "ORDERED", "RECEIVED", "PHOTO_AVAILABLE",
                   "MODEL_IDENTIFIED", "DOCUMENTATION_FOUND", "MEASURED", "REVIEW_REQUIRED",
                   "REVIEWED", "VERIFIED", "VERIFY_ON_ARRIVAL", "CONFLICT"}
REVIEW_STATES = {"NOT_REVIEWED", "REVIEW_REQUIRED", "REVIEWED", "CONFLICT"}
DRIVER_STATES = {"NOT_STARTED", "BLOCKED_ON_IDENTITY", "BLOCKED_ON_DATASHEET",
                 "BLOCKED_ON_MEASUREMENT", "READY_FOR_IMPLEMENTATION", "IMPLEMENTED",
                 "COMMISSIONING_REQUIRED"}
CAD_STATES = {"CANDIDATE", "PARAMETRIC", "MEASUREMENT_REQUIRED", "MATCH_REVIEW_REQUIRED",
              "MATCHED", "MISMATCH", "BLOCKED", "NOT_APPLICABLE"}
CANDIDATE_MATCHES = {"NONE", "POSSIBLE_MATCH", "MATCHED", "MISMATCH"}
ID_PATTERN = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$")

USER_FIELDS = {"inventory_id", "part_name", "manufacturer", "model_exact_variant", "quantity",
               "physical_status", "purchase_link_reference", "photo_reference",
               "storage_location", "notes"}
ENGINEERING_FIELDS = {"controller_ownership", "function", "interface_documented",
    "voltage_documented", "current_documented", "voltage_measured", "current_measured",
    "connector", "zie_logical_slot", "driver_extension", "hardware_profile_mapping",
    "cad_status", "dimensions_documented", "dimensions_measured", "weight_documented",
    "weight_measured", "safety_criticality", "verify_on_arrival",
    "decision_keep_replace_undecided", "evidence_source", "reviewer", "review_date",
    "review_state", "evidence_state", "candidate_match", "driver_state",
    "commissioning_dependencies", "measurement_requirements", "conflict_reason"}
RUNTIME_FIELDS = {"physical_status", "storage_location", "review_date", "review_state",
    "evidence_state", "candidate_match", "driver_state", "commissioning_dependencies",
    "measurement_requirements", "conflict_reason", "verified", "removed", "created_at",
    "updated_at", "history"}


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat()


def _text(value: Any, limit: int = 1000) -> str:
    if value is None:
        return ""
    if not isinstance(value, str) or len(value) > limit or "\x00" in value:
        raise ValueError("invalid or oversized text field")
    return value.strip()


class InventoryStore:
    """Revisioned JSON state plus exact-schema CSV import/export; no robot authority."""

    def __init__(self, root: Path, columns: list[str]):
        self.root = root.resolve()
        self.active_path = self.root / "active.json"
        self.previous_path = self.root / "previous.json"
        self.columns = tuple(columns)
        self._lock = threading.RLock()
        self.root.mkdir(parents=True, exist_ok=True)
        self._state = self._load()

    @staticmethod
    def empty() -> dict[str, Any]:
        return {"format": 1, "revision": 0, "last_saved": None, "items": []}

    def _load_file(self, path: Path) -> dict[str, Any]:
        raw = path.read_bytes()
        if len(raw) > MAX_BYTES:
            raise ValueError("inventory state exceeds 2 MiB")
        state = json.loads(raw.decode("utf-8"))
        self._validate_state(state)
        return state

    def _load(self) -> dict[str, Any]:
        if not self.active_path.exists():
            return self.empty()
        try:
            return self._load_file(self.active_path)
        except (OSError, UnicodeError, json.JSONDecodeError, ValueError) as exc:
            raise RuntimeError("active inventory is malformed; recovery required") from exc

    def _validate_state(self, state: Any) -> None:
        if not isinstance(state, dict) or set(state) != {"format", "revision", "last_saved", "items"}:
            raise ValueError("invalid inventory state shape")
        if state["format"] != 1 or not isinstance(state["revision"], int) or state["revision"] < 0:
            raise ValueError("invalid inventory revision")
        if not isinstance(state["items"], list) or len(state["items"]) > MAX_ROWS:
            raise ValueError("invalid inventory row count")
        seen: set[str] = set()
        for item in state["items"]:
            self._validate_item(item)
            if item["inventory_id"] in seen:
                raise ValueError("duplicate inventory ID")
            seen.add(item["inventory_id"])

    def _validate_item(self, item: Any) -> None:
        if not isinstance(item, dict) or not ID_PATTERN.fullmatch(str(item.get("inventory_id", ""))):
            raise ValueError("invalid inventory ID")
        if set(item) != set(self.columns) | RUNTIME_FIELDS:
            raise ValueError("invalid inventory item shape")
        if not _text(item.get("part_name"), 200):
            raise ValueError("part name is required")
        quantity = item.get("quantity")
        if not isinstance(quantity, int) or isinstance(quantity, bool) or not 1 <= quantity <= 10000:
            raise ValueError("quantity must be an integer from 1 to 10000")
        checks = (("physical_status", PHYSICAL_STATUSES), ("decision_keep_replace_undecided", DECISIONS),
                  ("controller_ownership", OWNERS), ("evidence_state", EVIDENCE_STATES),
                  ("review_state", REVIEW_STATES), ("driver_state", DRIVER_STATES),
                  ("cad_status", CAD_STATES), ("candidate_match", CANDIDATE_MATCHES))
        for key, allowed in checks:
            if item.get(key) not in allowed:
                raise ValueError(f"invalid {key}")
        if item.get("verified") is not False and item.get("verified") is not True:
            raise ValueError("invalid verification flag")
        for key, value in item.items():
            if isinstance(value, str):
                _text(value)
            elif isinstance(value, list) and key != "history":
                if len(value) > 32 or not all(isinstance(entry, str) and len(entry) <= 200 for entry in value):
                    raise ValueError("invalid bounded list")
            elif key == "history" and (len(value) > 20 or not all(isinstance(entry, dict) for entry in value)):
                raise ValueError("invalid bounded history")

    def _base_item(self, values: dict[str, Any]) -> dict[str, Any]:
        unexpected = set(values) - USER_FIELDS
        if unexpected:
            raise ValueError(f"unexpected intake fields: {sorted(unexpected)}")
        item = {key: "" for key in self.columns}
        item.update({
            "inventory_id": _text(values.get("inventory_id"), 64),
            "part_name": _text(values.get("part_name"), 200),
            "manufacturer": _text(values.get("manufacturer"), 200),
            "model_exact_variant": _text(values.get("model_exact_variant"), 200),
            "quantity": values.get("quantity"),
            "physical_status": values.get("physical_status", "UNKNOWN"),
            "purchase_link_reference": _text(values.get("purchase_link_reference")),
            "photo_reference": _text(values.get("photo_reference")),
            "storage_location": _text(values.get("storage_location"), 200),
            "notes": _text(values.get("notes")),
            "purchase_status": {"RECEIVED": "received", "ORDERED": "ordered",
                                "NOT_BOUGHT": "required", "UNKNOWN": ""}.get(values.get("physical_status", "UNKNOWN"), ""),
            "controller_ownership": "UNKNOWN", "decision_keep_replace_undecided": "UNDECIDED",
            "evidence_state": "REVIEW_REQUIRED", "review_state": "REVIEW_REQUIRED",
            "driver_state": "BLOCKED_ON_IDENTITY", "cad_status": "MEASUREMENT_REQUIRED",
            "candidate_match": "NONE", "review_date": "", "commissioning_dependencies": [],
            "measurement_requirements": [], "conflict_reason": "", "verified": False,
            "removed": False, "created_at": utc_now(), "updated_at": utc_now(), "history": [],
        })
        self._validate_item(item)
        return item

    def snapshot(self) -> dict[str, Any]:
        state = json.loads(json.dumps(self._state))
        state["previous_revision_available"] = self.previous_path.exists()
        state["active_count"] = sum(not item["removed"] for item in state["items"])
        return state

    def _save(self, items: list[dict[str, Any]], expected_revision: int, event: str) -> dict[str, Any]:
        with self._lock:
            if expected_revision != self._state["revision"]:
                raise RuntimeError("STALE_REVISION")
            next_state = {"format": 1, "revision": expected_revision + 1,
                          "last_saved": utc_now(), "items": items}
            self._validate_state(next_state)
            encoded = json.dumps(next_state, ensure_ascii=False, sort_keys=True, indent=2).encode("utf-8")
            if len(encoded) > MAX_BYTES:
                raise ValueError("inventory state exceeds 2 MiB")
            fd, temporary = tempfile.mkstemp(prefix="inventory-", suffix=".tmp", dir=self.root)
            try:
                with os.fdopen(fd, "wb") as stream:
                    stream.write(encoded); stream.flush(); os.fsync(stream.fileno())
                if self.active_path.exists():
                    backup_fd, backup_temporary = tempfile.mkstemp(prefix="previous-", suffix=".tmp", dir=self.root)
                    try:
                        with os.fdopen(backup_fd, "wb") as backup:
                            backup.write(self.active_path.read_bytes()); backup.flush(); os.fsync(backup.fileno())
                        os.replace(backup_temporary, self.previous_path)
                    finally:
                        if os.path.exists(backup_temporary): os.unlink(backup_temporary)
                os.replace(temporary, self.active_path)
            finally:
                if os.path.exists(temporary): os.unlink(temporary)
            self._state = next_state
        return {"revision": next_state["revision"], "state": "SAVED", "event": event,
                "robot_authority": "NONE"}

    def create(self, values: dict[str, Any], expected_revision: int) -> dict[str, Any]:
        item = self._base_item(values)
        if any(existing["inventory_id"] == item["inventory_id"] for existing in self._state["items"]):
            raise ValueError("duplicate inventory ID")
        result = self._save([*self._state["items"], item], expected_revision, "item_created")
        return {**result, "item": item}

    def update(self, inventory_id: str, values: dict[str, Any], expected_revision: int) -> dict[str, Any]:
        unexpected = set(values) - (USER_FIELDS - {"inventory_id"})
        if unexpected:
            raise ValueError(f"unexpected edit fields: {sorted(unexpected)}")
        items = json.loads(json.dumps(self._state["items"])); target = self._find(items, inventory_id)
        for key, value in values.items():
            target[key] = value if key == "quantity" else _text(value, 1000)
        if "physical_status" in values:
            target["purchase_status"] = {"RECEIVED": "received", "ORDERED": "ordered",
                "NOT_BOUGHT": "required", "UNKNOWN": ""}.get(values["physical_status"], "")
        target.update({"verified": False, "review_state": "REVIEW_REQUIRED",
                       "evidence_state": "REVIEW_REQUIRED", "updated_at": utc_now()})
        self._validate_item(target)
        result = self._save(items, expected_revision, "item_changed")
        return {**result, "item": target}

    def remove(self, inventory_id: str, expected_revision: int) -> dict[str, Any]:
        items = json.loads(json.dumps(self._state["items"])); target = self._find(items, inventory_id)
        target.update({"removed": True, "verified": False, "review_state": "REVIEW_REQUIRED",
                       "updated_at": utc_now()})
        return self._save(items, expected_revision, "item_tombstoned")

    @staticmethod
    def _find(items: list[dict[str, Any]], inventory_id: str) -> dict[str, Any]:
        for item in items:
            if item["inventory_id"] == inventory_id:
                return item
        raise KeyError("inventory item not found")

    def review(self, inventory_id: str, values: dict[str, Any], expected_revision: int,
               request_verification: bool = False) -> dict[str, Any]:
        unexpected = set(values) - ENGINEERING_FIELDS
        if unexpected:
            raise ValueError(f"unexpected engineering fields: {sorted(unexpected)}")
        items = json.loads(json.dumps(self._state["items"])); target = self._find(items, inventory_id)
        was_verified = target["verified"]
        for key, value in values.items():
            target[key] = value if isinstance(value, list) else _text(value)
        verified = False
        if request_verification:
            requirements = [target["physical_status"] == "RECEIVED", bool(target["manufacturer"]),
                bool(target["model_exact_variant"]), target["review_state"] == "REVIEWED",
                target["evidence_state"] == "REVIEWED", bool(target["evidence_source"]),
                bool(target["reviewer"]), bool(target["review_date"]), not target["removed"],
                not target["conflict_reason"], target["candidate_match"] != "MISMATCH"]
            if not all(requirements):
                raise ValueError("VERIFIED transition requirements are not satisfied")
            verified = True
            target["evidence_state"] = "VERIFIED"
        elif was_verified:
            target["evidence_state"] = "REVIEW_REQUIRED"
            target["review_state"] = "REVIEW_REQUIRED"
        target["verified"] = verified
        target["updated_at"] = utc_now()
        target["history"] = [*target.get("history", [])[-19:], {"at": utc_now(), "event": "engineering_review",
                              "decision": target["decision_keep_replace_undecided"]}]
        self._validate_item(target)
        result = self._save(items, expected_revision, "review_changed")
        return {**result, "item": target}

    def import_csv(self, content: bytes, expected_revision: int) -> dict[str, Any]:
        rows = self.preview_csv(content)["items"]
        return self._save(rows, expected_revision, "import_persisted")

    def preview_csv(self, content: bytes) -> dict[str, Any]:
        if not content or len(content) > MAX_BYTES:
            raise ValueError("inventory CSV is empty or exceeds 2 MiB")
        try: text = content.decode("utf-8-sig")
        except UnicodeDecodeError as exc: raise ValueError("inventory CSV must be UTF-8") from exc
        reader = csv.DictReader(io.StringIO(text))
        if tuple(reader.fieldnames or ()) != self.columns:
            raise ValueError("inventory CSV header does not match the canonical schema")
        items, seen = [], set()
        for number, row in enumerate(reader, start=2):
            if len(items) >= MAX_ROWS: raise ValueError("inventory CSV exceeds 500 rows")
            if None in row: raise ValueError(f"inventory CSV row {number} has unexpected columns")
            inventory_id = _text(row["inventory_id"], 64)
            if inventory_id in seen: raise ValueError("duplicate inventory ID")
            seen.add(inventory_id)
            try: quantity = int(row["quantity"])
            except (TypeError, ValueError) as exc: raise ValueError(f"invalid quantity at row {number}") from exc
            purchase_status = _text(row["purchase_status"]).lower()
            if purchase_status not in {"", "existing", "required", "ordered", "received", "quarantined", "returned"}:
                raise ValueError(f"invalid purchase_status at row {number}")
            role = _text(row["procurement_role"]).lower()
            if role not in {"", "existing", "required", "substitute_candidate"}:
                raise ValueError(f"invalid procurement_role at row {number}")
            physical = {"received": "RECEIVED", "ordered": "ORDERED", "required": "NOT_BOUGHT",
                        "": "UNKNOWN", "existing": "UNKNOWN", "quarantined": "UNKNOWN",
                        "returned": "UNKNOWN"}[purchase_status]
            base = self._base_item({"inventory_id": inventory_id, "part_name": row["part_name"],
                "manufacturer": row["manufacturer"], "model_exact_variant": row["model_exact_variant"],
                "quantity": quantity, "physical_status": physical,
                "purchase_link_reference": row["purchase_link_reference"],
                "photo_reference": row["photo_reference"], "storage_location": "", "notes": row["notes"]})
            for key in self.columns:
                base[key] = _text(row[key]) if key != "quantity" else quantity
            owner = {"esp32-s3": "ESP32", "esp32": "ESP32", "stm32": "STM32", "host": "HOST",
                     "passive": "PASSIVE", "protected_safety": "PROTECTED_SAFETY",
                     "unknown": "UNKNOWN", "": "UNKNOWN"}
            raw_owner = str(base["controller_ownership"]).lower()
            if raw_owner not in owner: raise ValueError(f"invalid controller_ownership at row {number}")
            base["controller_ownership"] = owner[raw_owner]
            decision = str(base["decision_keep_replace_undecided"]).upper()
            if decision and decision not in DECISIONS: raise ValueError(f"invalid decision at row {number}")
            base["decision_keep_replace_undecided"] = decision or "UNDECIDED"
            legacy_cad = {"absent":"BLOCKED", "envelope":"PARAMETRIC", "modelled":"CANDIDATE",
                          "fit_coupon_passed":"MATCH_REVIEW_REQUIRED", "received_fit_verified":"MATCHED",
                          "rework":"MISMATCH", "":"MEASUREMENT_REQUIRED"}
            cad = str(base["cad_status"]).upper()
            raw_cad = str(base["cad_status"]).lower()
            if cad not in CAD_STATES and raw_cad not in legacy_cad:
                raise ValueError(f"invalid cad_status at row {number}")
            base["cad_status"] = cad if cad in CAD_STATES else legacy_cad[raw_cad]
            base.update({"physical_status": physical, "verified": False, "review_state": "REVIEW_REQUIRED",
                         "evidence_state": "REVIEW_REQUIRED", "decision_keep_replace_undecided": "UNDECIDED"})
            self._validate_item(base); items.append(base)
        return {"mode": "IMPORT_PREVIEW", "count": len(items), "items": items,
                "physical_verification": "NOT_VERIFIED", "persisted": False,
                "conflicts": self.blockers(items)}

    def export_csv(self) -> bytes:
        output = io.StringIO(newline=""); writer = csv.DictWriter(output, fieldnames=self.columns, lineterminator="\n")
        writer.writeheader()
        for item in sorted((x for x in self._state["items"] if not x["removed"]), key=lambda x: x["inventory_id"]):
            writer.writerow({key: item.get(key, "") for key in self.columns})
        return output.getvalue().encode("utf-8")

    def rollback(self, expected_revision: int) -> dict[str, Any]:
        if expected_revision != self._state["revision"]: raise RuntimeError("STALE_REVISION")
        if not self.previous_path.exists(): raise RuntimeError("previous revision unavailable")
        previous = self._load_file(self.previous_path)
        return self._save(previous["items"], expected_revision, "rollback")

    def blockers(self, items: list[dict[str, Any]] | None = None) -> list[dict[str, str]]:
        results = []
        for item in items or self._state["items"]:
            if item["removed"]: continue
            checks = [(not item["model_exact_variant"], "EXACT_VARIANT_UNKNOWN"),
                      (item["controller_ownership"] == "UNKNOWN", "OWNERSHIP_UNRESOLVED"),
                      (not item["zie_logical_slot"], "LOGICAL_SLOT_UNRESOLVED"),
                      (item["driver_state"].startswith("BLOCKED"), "DRIVER_BLOCKED"),
                      (item["cad_status"] in {"BLOCKED", "MEASUREMENT_REQUIRED", "MATCH_REVIEW_REQUIRED"}, "CAD_BLOCKED"),
                      (not item["dimensions_measured"] and item["cad_status"] != "NOT_APPLICABLE", "MEASUREMENT_MISSING"),
                      (item["review_state"] != "REVIEWED", "REVIEW_REQUIRED"),
                      (item["decision_keep_replace_undecided"] == "UNDECIDED", "DECISION_REQUIRED"),
                      (bool(item["conflict_reason"]) or item["evidence_state"] == "CONFLICT", "CONFLICT")]
            for condition, code in checks:
                if condition: results.append({"inventory_id": item["inventory_id"], "code": code})
        return results[:5000]

    def reconciliation(self) -> dict[str, Any]:
        active = [json.loads(json.dumps(x)) for x in self._state["items"] if not x["removed"]]
        for item in active:
            item["reconciliation"] = {"exact_identity_known": bool(item["manufacturer"] and item["model_exact_variant"]),
                "physically_received": item["physical_status"] == "RECEIVED",
                "candidate_match": item["candidate_match"], "ownership_known": item["controller_ownership"] != "UNKNOWN",
                "logical_slot_known": bool(item["zie_logical_slot"]), "driver_known": bool(item["driver_extension"]),
                "profile_mapping_known": bool(item["hardware_profile_mapping"]),
                "cad_candidate_exists": item["cad_status"] not in {"BLOCKED", "NOT_APPLICABLE"},
                "measured_dimensions_available": bool(item["dimensions_measured"]),
                "electrical_interface_known": bool(item["interface_documented"] and item["voltage_documented"]),
                "safety_critical": item["safety_criticality"].lower() in {"critical", "high"},
                "review_required": item["review_state"] != "REVIEWED", "verified": item["verified"]}
        count = lambda predicate: sum(1 for item in active if predicate(item))
        return {"revision": self._state["revision"], "empty": not active,
            "summary": {"total_active": len(active), "received": count(lambda x:x["physical_status"]=="RECEIVED"),
                "ordered": count(lambda x:x["physical_status"]=="ORDERED"), "not_bought": count(lambda x:x["physical_status"]=="NOT_BOUGHT"),
                "unknown": count(lambda x:x["physical_status"]=="UNKNOWN"), "verified": count(lambda x:x["verified"]),
                "review_required": count(lambda x:x["review_state"]!="REVIEWED"), "conflict": count(lambda x:x["evidence_state"]=="CONFLICT" or bool(x["conflict_reason"])),
                "keep": count(lambda x:x["decision_keep_replace_undecided"]=="KEEP"), "replace": count(lambda x:x["decision_keep_replace_undecided"]=="REPLACE"),
                "undecided": count(lambda x:x["decision_keep_replace_undecided"]=="UNDECIDED"), "verify_on_arrival": count(lambda x:str(x["verify_on_arrival"]).lower()=="yes"),
                "safety_critical_unresolved": count(lambda x:x["safety_criticality"].lower() in {"critical","high"} and not x["verified"]),
                "cad_blocked": count(lambda x:x["cad_status"] in {"BLOCKED","MEASUREMENT_REQUIRED","MATCH_REVIEW_REQUIRED"}),
                "driver_blocked": count(lambda x:x["driver_state"].startswith("BLOCKED")),
                "driver_ready": count(lambda x:x["driver_state"]=="READY_FOR_IMPLEMENTATION"),
                "driver_implemented": count(lambda x:x["driver_state"]=="IMPLEMENTED"),
                "identity_known": count(lambda x:x["reconciliation"]["exact_identity_known"]),
                "cad_matched": count(lambda x:x["cad_status"]=="MATCHED"),
                "measurement_required": count(lambda x:not x["dimensions_measured"] and x["cad_status"]!="NOT_APPLICABLE")},
            "items": active, "blockers": self.blockers(active),
            "commissioning": {"physical_gates_passed": 0, "inventory_can_pass_gate": False,
                "state": "READY_FOR_PHYSICAL_TEST" if active and all(x["verified"] for x in active) else "BLOCKED",
                "dependency_catalog": {"battery/protection":[3,4], "ESP32 identity":[5], "STM32 identity":[6],
                    "controller link hardware":[7,8], "sensors":[9], "motors/drivers/servos":[10,11],
                    "safety sensors":[12], "camera/display/audio":[13], "drivetrain":[14]}},
            "phase2b2": {"state": "WAITING_FOR_VERIFIED_INPUTS", "categories": ["exact ESP32 variant", "exact STM32 implementation", "CAN transceiver", "physical topology and cable characteristics", "controller clocks", "real rates and bursts", "queue and ISR evidence", "measured safe-stop requirement", "payload and fragmentation requirements"]},
            "candidate_hardware": {"source": "DESIGN_DOCUMENTATION_ONLY", "automatically_imported": False,
                                   "automatic_match_authority": False}}
