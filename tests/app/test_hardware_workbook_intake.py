from pathlib import Path

from app.inventory.reconciliation import build_reports
from app.inventory.store import InventoryStore
from app.server.control_center import inventory_schema
from scripts import import_hardware_master_inventory as intake


def workbook_rows():
    header = ["ID", "Category", "Part", "Model / Marking", "Role", "Qty", "Unit",
        "Status / Evidence", "Confidence", "Reference specifications", "Interface",
        "Power / Voltage", "Zi-E use", "Photo / SKU evidence", "Software need",
        "Verification & safety notes"]
    rows = [header]
    quantities = [1] * 33
    quantities[30] = 137  # 31 supported rows total 167; candidates are separate.
    for number in range(1, 34):
        status = "Photo-confirmed"
        model = f"MODEL-{number:03d}"
        if number == 2:
            status, model = "Ordered; awaiting arrival", "ESP32-S3 N16R8 with OV5640 5 MP camera"
        elif number == 7:
            model = "VL53L0X / GY-53 / CJMCU style"
        elif number == 8:
            model = "WS2812/WS2811 family"
        elif number == 10:
            model = "5 V / 3 A seller description"
        elif number == 17:
            status = "Receipt-confirmed; photo not clearly matched"
        elif number >= 32:
            status = "Seller-reference only; purchase unconfirmed"
        rows.append([f"HW-{number:03d}", "Category", f"Part {number}", model, "Primary",
            str(quantities[number - 1]), "piece", status, "Medium", "family reference",
            "documented interface", "documented voltage", "semantic function",
            "Photo set; SKU 123", "possible library", "verify physical article"])
    return rows


def canonical_content(monkeypatch, tmp_path: Path):
    monkeypatch.setattr(intake, "_read_rows", lambda source, sheet: workbook_rows())
    return intake.convert(tmp_path / intake.SOURCE_FILENAME)


def test_supported_workbook_rows_import_without_verification(monkeypatch, tmp_path):
    content = canonical_content(monkeypatch, tmp_path)
    store = InventoryStore(tmp_path / "inventory", inventory_schema())
    preview = store.preview_csv(content)
    assert len(preview["items"]) == 31
    assert sum(item["quantity"] for item in preview["items"]) == 167
    assert all(item["verified"] is False for item in preview["items"])
    assert all(item["review_state"] == "REVIEW_REQUIRED" for item in preview["items"])
    assert {item["inventory_id"] for item in preview["items"]}.isdisjoint({"HW-032", "HW-033"})


def test_ordered_camera_and_receipt_only_meter_preserve_physical_boundaries(monkeypatch, tmp_path):
    items = InventoryStore(tmp_path / "inventory", inventory_schema()).preview_csv(
        canonical_content(monkeypatch, tmp_path))["items"]
    by_id = {item["inventory_id"]: item for item in items}
    camera = by_id["HW-002"]
    assert camera["physical_status"] == "ORDERED"
    assert camera["verify_on_arrival"] == "yes"
    assert camera["verified"] is False and not camera["voltage_measured"]
    assert "provisional procurement references" in camera["notes"]
    meter = by_id["HW-017"]
    assert meter["physical_status"] == "UNKNOWN"
    assert "physical receipt is unresolved" in meter["notes"]


def test_draft_pin_sheets_cannot_freeze_wiring_or_advance_readiness(monkeypatch, tmp_path):
    items = InventoryStore(tmp_path / "inventory", inventory_schema()).preview_csv(
        canonical_content(monkeypatch, tmp_path))["items"]
    report = build_reports(items)
    assert report["cad_readiness"]["state"] == "CAD_EVIDENCE_INCOMPLETE"
    assert report["electrical_readiness"]["state"] == "ELECTRICAL_EVIDENCE_INCOMPLETE"
    assert report["esp32_readiness"]["state"] == "WAITING_FOR_EVIDENCE"
    assert report["stm32_readiness"]["state"] == "WAITING_FOR_EVIDENCE"
    assert report["phase2b2_inputs_required"]["state"] == "WAITING_FOR_VERIFIED_INPUTS"
    assert report["first_power_readiness"]["physical_power_authorized"] is False
    assert report["commissioning_prerequisites"]["physical_passes"] == 0
    assert report["robot_authority"] == "NONE"
    assert all(not item["zie_logical_slot"] and not item["hardware_profile_mapping"] for item in items)


def test_actual_variants_do_not_silently_satisfy_different_candidates(monkeypatch, tmp_path):
    items = InventoryStore(tmp_path / "inventory", inventory_schema()).preview_csv(
        canonical_content(monkeypatch, tmp_path))["items"]
    report = build_reports(items)
    range_requirement = next(row for row in report["required_hardware"] if row["requirement_id"] == "range.head")
    belly_requirement = next(row for row in report["required_hardware"] if row["requirement_id"] == "ui.belly")
    assert range_requirement["purchased_part_exists"] is False
    assert belly_requirement["purchased_part_exists"] is False
    assert range_requirement["coverage_status"] == "MISSING_PURCHASE"
    assert belly_requirement["coverage_status"] == "MISSING_PURCHASE"


def test_boundary_annotations_quarantine_mismatches_without_authority(monkeypatch, tmp_path):
    store = InventoryStore(tmp_path / "inventory", inventory_schema())
    store.import_csv(canonical_content(monkeypatch, tmp_path), 0)
    revision = 1
    for inventory_id in ("HW-007", "HW-008"):
        result = store.review(inventory_id, intake.BOUNDARY_ANNOTATIONS[inventory_id], revision)
        revision = result["revision"]
    report = build_reports(store.reconciliation()["items"])
    for inventory_id in ("HW-007", "HW-008"):
        record = next(row for row in report["reconciliation_records"] if row["inventory_id"] == inventory_id)
        assert record["candidate_match"] == "MISMATCH"
        assert record["quarantined"] is True and record["readiness"] == "BLOCKED"
        assert record["purchased"]["verified"] is False
    assert report["commissioning_prerequisites"]["physical_passes"] == 0
    assert report["robot_authority"] == "NONE"


def test_first_bench_report_allows_evidence_work_only(monkeypatch, tmp_path):
    store = InventoryStore(tmp_path / "inventory", inventory_schema())
    store.import_csv(canonical_content(monkeypatch, tmp_path), 0)
    report = build_reports(store.reconciliation()["items"])
    bench = report["first_bench_readiness"]
    assert bench["state"] == "EVIDENCE_COLLECTION_ONLY"
    assert bench["first_power_authorized"] is False
    assert bench["powered_subsystem_test_authorized"] is False
    assert bench["robot_authority"] == "NONE"
    assert bench["power"] == {"state":"BLOCKED_UNVERIFIED_POWER_HARDWARE",
        "inventory_ids":["HW-010"], "other_received_regulators":[],
        "candidate_only_not_owned":["HW-033 XL4015E"]}
    assert bench["camera"]["state"] == "BLOCKED_UNTIL_ARRIVAL_AND_REVIEW"
    assert bench["camera"]["seller_maps"] == "PROVISIONAL_ONLY"
    assert bench["wiring"]["verified_physical_pin_mappings"] == []
    assert bench["stm32_motion_can"]["phase2b2"] == "WAITING_FOR_VERIFIED_INPUTS"
    assert bench["stm32_motion_can"]["commissioning_passes"] == 0


def test_first_bench_item_plans_are_exact_and_feed_what_i_need(monkeypatch, tmp_path):
    items = InventoryStore(tmp_path / "inventory", inventory_schema()).preview_csv(
        canonical_content(monkeypatch, tmp_path))["items"]
    report = build_reports(items)
    plans = {row["inventory_id"]:row for row in report["first_bench_readiness"]["item_plans"]}
    assert plans["HW-010"]["bench_status"] == "DO_NOT_POWER_OR_CONNECT"
    assert "isolation or non-isolation evidence" in plans["HW-010"]["evidence_to_provide"]
    assert plans["HW-002"]["physical_status"] == "ORDERED"
    assert plans["HW-002"]["powered_test_authorized"] is False
    assert "OV5640 sensor marking close-up" in plans["HW-002"]["evidence_to_provide"]
    assert plans["HW-003"]["bench_status"] == "DO_NOT_CONNECT_YET"
    assert plans["HW-007"]["bench_status"] == "DO_NOT_CONNECT_YET_CONFLICT"
    assert plans["HW-017"]["bench_status"] == "BLOCKED_PENDING_PHYSICAL_MATCH"
    requests = [row for row in report["user_input_required"]
                if row.get("purpose") == "FIRST_BENCH_EVIDENCE_CLOSURE"]
    assert {row["inventory_id"] for row in requests} == set(plans)
    assert all(row["evidence_needed"] for row in requests)
