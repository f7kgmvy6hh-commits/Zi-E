import csv
import io
import json
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor

import pytest
from fastapi.testclient import TestClient

from app.inventory.store import InventoryStore, MAX_BYTES, MAX_ROWS, _spreadsheet_safe
from app.server.control_center import inventory_schema
from app.server.main import create_app
from tests.app.test_server import TOKEN, settings


HEADERS = {"Authorization": f"Bearer {TOKEN}"}


def intake(inventory_id="P1-001", **overrides):
    value = {"inventory_id": inventory_id, "part_name": "Actual user-entered part",
             "manufacturer": "", "model_exact_variant": "", "quantity": 1,
             "physical_status": "UNKNOWN", "purchase_link_reference": "",
             "photo_reference": "", "storage_location": "", "notes": ""}
    value.update(overrides)
    return value


def store(tmp_path):
    return InventoryStore(tmp_path / "inventory", inventory_schema())


def test_canonical_schema_remains_exactly_32_columns():
    columns = inventory_schema()
    assert len(columns) == 32
    assert columns[0] == "inventory_id" and columns[-1] == "date_received"


def csv_bytes(rows):
    output = io.StringIO(); writer = csv.DictWriter(output, fieldnames=inventory_schema())
    writer.writeheader(); writer.writerows(rows); return output.getvalue().encode("utf-8")


def canonical_row(inventory_id="P1-001", **overrides):
    row = {key: "" for key in inventory_schema()}
    row.update({"inventory_id": inventory_id, "part_name": "Piñon received part",
                "quantity": "1", "purchase_status": "received"})
    row.update(overrides); return row


def test_empty_create_persist_restart_and_revision_backup(tmp_path):
    first = store(tmp_path)
    assert first.reconciliation()["empty"] is True
    created = first.create(intake(), 0)
    assert created["revision"] == 1 and created["item"]["verified"] is False
    assert created["item"]["review_state"] == "REVIEW_REQUIRED"
    edited = first.update("P1-001", {"physical_status": "RECEIVED", "model_exact_variant": "user label"}, 1)
    assert edited["revision"] == 2 and first.previous_path.exists()
    restarted = store(tmp_path)
    assert restarted.snapshot()["revision"] == 2
    assert restarted.reconciliation()["items"][0]["physical_status"] == "RECEIVED"
    assert restarted.reconciliation()["items"][0]["verified"] is False


@pytest.mark.parametrize("quantity", [0, -1, 10001, True, "1"])
def test_quantity_validation(quantity, tmp_path):
    with pytest.raises(ValueError, match="quantity"):
        store(tmp_path).create(intake(quantity=quantity), 0)


@pytest.mark.parametrize("status", ["received", "BOUGHT", "VERIFIED", ""])
def test_invalid_physical_status_rejected(status, tmp_path):
    with pytest.raises(ValueError, match="physical_status"):
        store(tmp_path).create(intake(physical_status=status), 0)


def test_duplicate_id_stale_update_and_tombstone_preserve_history(tmp_path):
    value = store(tmp_path); value.create(intake(), 0)
    with pytest.raises(ValueError, match="duplicate"):
        value.create(intake(), 1)
    with pytest.raises(RuntimeError, match="STALE_REVISION"):
        value.update("P1-001", {"notes": "stale"}, 0)
    value.remove("P1-001", 1)
    snapshot = value.snapshot()
    assert snapshot["items"][0]["removed"] is True
    assert snapshot["items"][0]["verified"] is False
    assert value.reconciliation()["summary"]["total_active"] == 0


def test_csv_import_export_utf8_round_trip_never_verifies(tmp_path):
    value = store(tmp_path); content = csv_bytes([canonical_row(photo_reference="photo-ref")])
    preview = value.preview_csv(content)
    assert preview["persisted"] is False and preview["items"][0]["verified"] is False
    value.import_csv(content, 0)
    exported = value.export_csv()
    assert "Piñon" in exported.decode("utf-8")
    second = InventoryStore(tmp_path / "second", inventory_schema())
    second.import_csv(exported, 0)
    assert second.export_csv() == exported
    assert second.reconciliation()["summary"]["verified"] == 0


def test_csv_export_neutralizes_formulas_without_mutating_inventory(tmp_path):
    value = store(tmp_path)
    payload = "=HYPERLINK(\"https://invalid.example\",\"open\")"
    value.create(intake(notes=payload), 0)
    exported = list(csv.DictReader(io.StringIO(value.export_csv().decode("utf-8"))))
    assert exported[0]["notes"].startswith("'=")
    assert value.snapshot()["items"][0]["notes"] == payload


@pytest.mark.parametrize("value", ["=formula", "+formula", "-formula", "@formula", " @formula",
                                   "\tformula", "\rformula", "\nformula", " \tformula", " \rformula"])
def test_spreadsheet_formula_prefixes_are_neutralized(value):
    assert _spreadsheet_safe(value).startswith("'")


@pytest.mark.parametrize("value", [None, 0, -12, 3.5, False])
def test_spreadsheet_export_does_not_coerce_non_strings(value):
    assert _spreadsheet_safe(value) is value


def test_quarantined_import_is_physically_received_but_never_verified(tmp_path):
    preview = store(tmp_path).preview_csv(csv_bytes([canonical_row(purchase_status="quarantined")]))
    assert preview["items"][0]["physical_status"] == "RECEIVED"
    assert preview["items"][0]["verified"] is False


def test_csv_rejects_header_malformed_duplicate_oversize_and_excess_rows(tmp_path):
    value = store(tmp_path)
    with pytest.raises(ValueError, match="header"):
        value.preview_csv(b"wrong,header\nx,y\n")
    malformed = csv_bytes([canonical_row()]).decode() + ",unexpected\n"
    with pytest.raises(ValueError): value.preview_csv(malformed.encode())
    with pytest.raises(ValueError, match="duplicate"):
        value.preview_csv(csv_bytes([canonical_row(), canonical_row()]))
    with pytest.raises(ValueError, match="2 MiB"):
        value.preview_csv(b"x" * (MAX_BYTES + 1))
    rows = [canonical_row(f"P1-{number:04d}") for number in range(MAX_ROWS + 1)]
    with pytest.raises(ValueError, match="500 rows"): value.preview_csv(csv_bytes(rows))


@pytest.mark.parametrize(("field", "bad"), [("purchase_status","recieved"),
    ("procurement_role","primary"), ("controller_ownership","ESP33"),
    ("decision_keep_replace_undecided","MAYBE"), ("cad_status","FITS")])
def test_csv_rejects_unknown_controlled_values(field, bad, tmp_path):
    with pytest.raises(ValueError, match="invalid"):
        store(tmp_path).preview_csv(csv_bytes([canonical_row(**{field:bad})]))


def test_verification_requires_explicit_reviewed_evidence(tmp_path):
    value = store(tmp_path)
    value.create(intake(manufacturer="Maker", model_exact_variant="Exact", physical_status="RECEIVED",
                        photo_reference="photo only"), 0)
    for change in ({"evidence_state":"PHOTO_AVAILABLE"}, {"candidate_match":"MATCHED"},
                   {"review_state":"REVIEWED", "reviewer":"Engineer", "review_date":"2026-08-29"}):
        with pytest.raises(ValueError, match="VERIFIED transition"):
            value.review("P1-001", change, 1, True)
    reviewed = value.review("P1-001", {"evidence_state":"REVIEWED", "review_state":"REVIEWED",
        "evidence_source":"photo+label+datasheet review", "reviewer":"Engineer",
        "review_date":"2026-08-29", "candidate_match":"POSSIBLE_MATCH"}, 1, True)
    assert reviewed["item"]["verified"] is True
    assert reviewed["item"]["candidate_match"] == "POSSIBLE_MATCH"


def test_conflict_or_candidate_mismatch_blocks_verification_and_review_revokes_cleanly(tmp_path):
    value = store(tmp_path); value.create(intake(manufacturer="Maker", model_exact_variant="Exact",
        physical_status="RECEIVED"), 0)
    base = {"evidence_state":"REVIEWED", "review_state":"REVIEWED", "evidence_source":"reviewed source",
            "reviewer":"Engineer", "review_date":"2026-08-29"}
    with pytest.raises(ValueError, match="VERIFIED transition"):
        value.review("P1-001", {**base, "conflict_reason":"identity conflict"}, 1, True)
    with pytest.raises(ValueError, match="VERIFIED transition"):
        value.review("P1-001", {**base, "candidate_match":"MISMATCH"}, 1, True)
    verified = value.review("P1-001", base, 1, True)
    assert verified["item"]["evidence_state"] == "VERIFIED"
    revoked = value.review("P1-001", {"decision_keep_replace_undecided":"UNDECIDED"}, 2)
    assert revoked["item"]["verified"] is False
    assert revoked["item"]["evidence_state"] == "REVIEW_REQUIRED"
    assert revoked["item"]["review_state"] == "REVIEW_REQUIRED"


def test_conflict_safety_and_reconciliation_are_explicit(tmp_path):
    value = store(tmp_path); value.create(intake(), 0)
    result = value.review("P1-001", {"evidence_state":"CONFLICT", "review_state":"CONFLICT",
        "conflict_reason":"label differs from order", "safety_criticality":"critical",
        "controller_ownership":"UNKNOWN", "decision_keep_replace_undecided":"UNDECIDED"}, 1)
    assert result["item"]["verified"] is False
    report = value.reconciliation()
    assert report["summary"]["conflict"] == 1
    assert report["summary"]["safety_critical_unresolved"] == 1
    assert report["commissioning"]["physical_gates_passed"] == 0
    assert report["candidate_hardware"]["automatically_imported"] is False
    assert report["phase2b2"]["state"] == "WAITING_FOR_VERIFIED_INPUTS"


def test_rollback_uses_only_known_previous_revision(tmp_path):
    value = store(tmp_path)
    with pytest.raises(RuntimeError, match="unavailable"): value.rollback(0)
    value.create(intake(notes="one"), 0); value.update("P1-001", {"notes":"two"}, 1)
    rolled = value.rollback(2)
    assert rolled["revision"] == 3
    assert value.snapshot()["items"][0]["notes"] == "one"
    with pytest.raises(RuntimeError, match="STALE_REVISION"): value.rollback(2)


def test_malformed_active_state_fails_closed(tmp_path):
    root = tmp_path / "inventory"; root.mkdir(); (root / "active.json").write_text("{}", encoding="utf-8")
    with pytest.raises(RuntimeError, match="recovery required"):
        InventoryStore(root, inventory_schema())


def test_failed_atomic_replace_preserves_previous_active_state(tmp_path, monkeypatch):
    value = store(tmp_path); value.create(intake(notes="good"), 0)
    real_replace = __import__("os").replace
    def fail_active(source, destination):
        if Path(destination) == value.active_path:
            raise OSError("simulated replace failure")
        return real_replace(source, destination)
    monkeypatch.setattr("app.inventory.store.os.replace", fail_active)
    with pytest.raises(OSError, match="simulated"):
        value.update("P1-001", {"notes":"must not replace"}, 1)
    monkeypatch.setattr("app.inventory.store.os.replace", real_replace)
    restarted = InventoryStore(value.root, inventory_schema())
    assert restarted.snapshot()["revision"] == 1
    assert restarted.snapshot()["items"][0]["notes"] == "good"


def test_concurrent_same_revision_allows_only_one_writer(tmp_path):
    value = store(tmp_path)
    def create(number):
        try: return value.create(intake(f"P1-00{number}"), 0)["revision"]
        except RuntimeError as exc: return str(exc)
    with ThreadPoolExecutor(max_workers=2) as pool:
        results = list(pool.map(create, (1, 2)))
    assert results.count(1) == 1
    assert results.count("STALE_REVISION") == 1
    assert value.snapshot()["revision"] == 1
    assert value.reconciliation()["summary"]["total_active"] == 1


def test_api_mutations_audit_none_authority_and_no_path_surface(tmp_path):
    app = create_app(settings(tmp_path), inventory_root=tmp_path / "owned")
    with TestClient(app) as client:
        body = {"expected_revision":0, **intake()}
        response = client.post("/api/inventory/items", headers=HEADERS, json=body)
        assert response.status_code == 200 and response.json()["robot_authority"] == "NONE"
        assert client.patch("/api/inventory/items/P1-001", headers=HEADERS,
            json={"expected_revision":0,"notes":"stale"}).status_code == 409
        assert client.post("/api/inventory/items", headers=HEADERS,
            json={**body, "path":"../escape"}).status_code == 422
        status = client.get("/api/inventory", headers=HEADERS).json()
        assert status["commissioning"]["physical_gates_passed"] == 0
        events = [x for x in client.app.state.events.history() if x["type"] == "inventory.mutation"]
        assert events[-1]["payload"]["robot_authority"] == "NONE"
        paths = {route.path.lower() for route in client.app.routes}
        assert not any("upload" in path or "filesystem" in path for path in paths)


def test_api_import_preview_commit_export_and_restart(tmp_path):
    root = tmp_path / "owned"; content = csv_bytes([canonical_row()])
    with TestClient(create_app(settings(tmp_path), inventory_root=root)) as client:
        preview = client.post("/api/inventory/import/preview", headers=HEADERS, content=content)
        assert preview.json()["persisted"] is False
        committed = client.post("/api/inventory/import/commit?expected_revision=0", headers=HEADERS, content=content)
        assert committed.status_code == 200
        exported = client.get("/api/inventory/export", headers=HEADERS)
        exported_rows = list(csv.DictReader(io.StringIO(exported.content.decode("utf-8"))))
        assert tuple(exported_rows[0]) == tuple(inventory_schema())
        assert exported_rows[0]["inventory_id"] == "P1-001"
    with TestClient(create_app(settings(tmp_path), inventory_root=root)) as client:
        assert client.get("/api/inventory", headers=HEADERS).json()["summary"]["total_active"] == 1
