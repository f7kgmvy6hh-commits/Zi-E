import pytest
from fastapi.testclient import TestClient

from app.inventory.reconciliation import build_reports
from app.inventory.store import InventoryStore
from app.server.control_center import inventory_schema
from app.server.main import create_app
from tests.app.test_server import TOKEN, settings


def item(**overrides):
    value = {
        "inventory_id": "ACT-001", "part_name": "User-owned controller",
        "manufacturer": "Espressif", "model_exact_variant": "ESP32-S3-WROOM-1-N16R8",
        "quantity": 1, "physical_status": "RECEIVED", "purchase_link_reference": "order-1",
        "photo_reference": "label-photo", "storage_location": "bench", "notes": "",
        "evidence_source": "", "review_state": "REVIEW_REQUIRED", "evidence_state": "PHOTO_AVAILABLE",
        "candidate_match": "NONE", "decision_keep_replace_undecided": "UNDECIDED",
        "controller_ownership": "UNKNOWN", "zie_logical_slot": "", "driver_extension": "",
        "driver_state": "BLOCKED_ON_IDENTITY", "cad_status": "MEASUREMENT_REQUIRED",
        "dimensions_measured": "", "weight_measured": "", "voltage_documented": "",
        "current_documented": "", "interface_documented": "", "connector": "",
        "safety_criticality": "", "conflict_reason": "", "verified": False,
    }
    value.update(overrides)
    return value


def test_empty_inventory_produces_real_intake_queue_without_seeding_candidates():
    report = build_reports([])
    assert report["actual_active_inventory_count"] == 0
    assert report["candidate_catalog"]["documented_primary_count"] == 34
    assert report["candidate_catalog"]["purchased_inventory_seeded"] is False
    assert report["reconciliation_records"] == []
    assert all(not row["purchased_part_exists"] for row in report["required_hardware"])
    assert report["user_input_required"]
    assert report["phase2b2_inputs_required"]["state"] == "WAITING_FOR_VERIFIED_INPUTS"
    assert report["commissioning_prerequisites"]["physical_passes"] == 0
    assert report["first_power_readiness"]["physical_power_authorized"] is False
    assert report["robot_authority"] == "NONE"


def test_exact_and_fuzzy_candidate_results_remain_non_authoritative():
    exact_report = build_reports([item()])
    exact = exact_report["reconciliation_records"][0]
    fuzzy = build_reports([item(model_exact_variant="ESP32 S3 WROOM N16R8")])["reconciliation_records"][0]
    assert exact["candidate_match"] == "MATCH_REVIEW_REQUIRED"
    assert exact["identity_status"] == "IDENTIFIED" and exact["purchased"]["verified"] is False
    assert all(not row["purchased_part_exists"] for row in exact_report["required_hardware"])
    assert fuzzy["candidate_match"] in {"POSSIBLE_MATCH", "NO_CANDIDATE"}
    assert fuzzy["candidate_match"] != "MATCHED"


def test_conflict_quarantines_and_blocks_readiness():
    record = build_reports([item(candidate_match="CONFLICT", conflict_reason="labels disagree")])["reconciliation_records"][0]
    assert record["quarantined"] is True
    assert record["readiness"] == "BLOCKED"
    assert "DESIGN_CONFLICT" in record["blockers"]


def test_next_action_and_readiness_require_actual_evidence():
    report = build_reports([item(model_exact_variant="", manufacturer="")])
    record = report["reconciliation_records"][0]
    assert record["next_action"] == "Photograph and transcribe the exact model label."
    assert "BLOCKED_ON_IDENTITY" in record["blockers"]
    assert report["cad_readiness"]["state"] == "CAD_EVIDENCE_INCOMPLETE"
    assert report["electrical_readiness"]["state"] == "ELECTRICAL_EVIDENCE_INCOMPLETE"
    assert all(gate["physical_result"] == "NOT_TESTED" for gate in report["commissioning_prerequisites"]["gates"])


def test_unreviewed_typed_fields_cannot_advance_engineering_readiness():
    report = build_reports([item(candidate_match="MATCHED",
        candidate_reference="ESP32-S3-WROOM-1-N16R8 candidate",
        dimensions_measured="typed dimensions", cad_status="MATCHED",
        voltage_documented="3.3V", current_documented="typed", interface_documented="SPI",
        connector="header", driver_state="READY_FOR_IMPLEMENTATION")])
    row = next(x for x in report["required_hardware"] if x["requirement_id"] == "compute.esp32")
    assert row["purchased_part_exists"] is False
    assert row["cad_ready"] is False and row["electrical_ready"] is False and row["driver_ready"] is False


def test_verified_reviewed_evidence_can_reach_engineering_readiness_only():
    report = build_reports([item(candidate_match="MATCHED", review_state="REVIEWED",
        evidence_state="VERIFIED", verified=True, decision_keep_replace_undecided="KEEP",
        candidate_reference="ESP32-S3-WROOM-1-N16R8 candidate",
        dimensions_measured="reviewed dimensions", cad_status="MATCHED",
        voltage_documented="reviewed", current_documented="reviewed", interface_documented="reviewed SPI",
        connector="reviewed header", evidence_source="reviewed datasheet", controller_ownership="ESP32",
        zie_logical_slot="controller.multimedia")])
    row = next(x for x in report["required_hardware"] if x["requirement_id"] == "compute.esp32")
    assert row["cad_ready"] and row["electrical_ready"] and row["driver_ready"]
    assert report["commissioning_prerequisites"]["physical_passes"] == 0


def test_keep_and_replace_are_explicit_guarded_decisions(tmp_path):
    store = InventoryStore(tmp_path / "inventory", inventory_schema())
    store.create({key: item()[key] for key in ("inventory_id", "part_name", "manufacturer",
        "model_exact_variant", "quantity", "physical_status", "purchase_link_reference",
        "photo_reference", "storage_location", "notes")}, 0)
    with pytest.raises(ValueError, match="KEEP requires"):
        store.review("ACT-001", {"decision_keep_replace_undecided": "KEEP"}, 1)
    with pytest.raises(ValueError, match="REPLACE requires"):
        store.review("ACT-001", {"decision_keep_replace_undecided": "REPLACE"}, 1)
    replaced = store.review("ACT-001", {"decision_keep_replace_undecided": "REPLACE",
        "decision_reason_code": "EXACT_VARIANT_MISMATCH", "decision_notes": "reviewed variant differs"}, 1)
    assert replaced["item"]["decision_keep_replace_undecided"] == "REPLACE"
    assert replaced["item"]["verified"] is False


def test_material_identity_edit_revokes_verification_and_records_history(tmp_path):
    store = InventoryStore(tmp_path / "inventory", inventory_schema())
    intake = {key: item()[key] for key in ("inventory_id", "part_name", "manufacturer",
        "model_exact_variant", "quantity", "physical_status", "purchase_link_reference",
        "photo_reference", "storage_location", "notes")}
    store.create(intake, 0)
    verified = store.review("ACT-001", {"review_state": "REVIEWED", "evidence_state": "REVIEWED",
        "evidence_source": "label plus datasheet", "reviewer": "engineer", "review_date": "2026-08-29"}, 1, True)
    assert verified["item"]["verified"] is True
    changed = store.update("ACT-001", {"model_exact_variant": "different-model"}, 2)
    assert changed["item"]["verified"] is False
    assert changed["item"]["candidate_match"] == "NONE"
    assert changed["item"]["decision_keep_replace_undecided"] == "UNDECIDED"
    assert changed["item"]["history"][-1]["verification_revoked"] is True


def test_reconciliation_api_is_authenticated_closed_and_has_no_robot_authority(tmp_path):
    headers = {"Authorization": f"Bearer {TOKEN}"}
    with TestClient(create_app(settings(tmp_path), inventory_root=tmp_path / "inventory")) as client:
        assert client.get("/api/reconciliation").status_code == 401
        report = client.get("/api/reconciliation", headers=headers).json()
        assert report["actual_active_inventory_count"] == 0
        assert report["robot_authority"] == "NONE"
        assert client.get("/api/reconciliation/reports/../../secrets", headers=headers).status_code == 404
        assert client.get("/api/reconciliation/reports/not-allowlisted", headers=headers).status_code == 404
