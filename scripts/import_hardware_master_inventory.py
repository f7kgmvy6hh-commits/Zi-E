"""One-time Zi-E hardware workbook conversion into the 0.04 inventory store.

This intentionally supports one evidence workbook layout, excludes seller-only
candidates, and never reviews, verifies, commissions, or grants robot authority.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import io
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET
import zipfile

REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPOSITORY_ROOT))

from app.inventory.store import InventoryStore  # noqa: E402
from app.server.control_center import inventory_schema  # noqa: E402

SOURCE_FILENAME = "Zi-E_Hardware_Master_Inventory.xlsx"
SUPPORTED_IDS = tuple(f"HW-{number:03d}" for number in range(1, 32))
CANDIDATE_ONLY_IDS = ("HW-032", "HW-033")
EXPECTED_QUANTITY = 167
BOUNDARY_ANNOTATIONS = {
    "HW-002": {
        "implementation_blocker": "Await physical board/revision/sensor comparison; seller camera GPIO and firmware pin sheets are provisional only.",
        "measurement_requirements": ["front/back/revision/sensor/pinout arrival evidence"],
    },
    "HW-003": {
        "electrical_blocker": "Verify TFT touch-controller identity, VCC, and logic levels before wiring.",
    },
    "HW-006": {
        "electrical_blocker": "Verify MAX9814 labels, VIN, output bias, and ESP32 ADC compatibility.",
    },
    "HW-007": {
        "candidate_match": "MISMATCH",
        "candidate_reference": "VL53L1X head rangefinder",
        "mismatch_reason": "Received evidence identifies VL53L0X/GY-53/CJMCU style, not the VL53L1X design candidate.",
        "conflict_reason": "Explicit KEEP/REPLACE/UNDECIDED reconciliation is required; no silent substitution.",
        "electrical_blocker": "Verify breakout VIN and level shifting before wiring.",
    },
    "HW-008": {
        "candidate_match": "MISMATCH",
        "candidate_reference": "Adafruit IS31FL3741 13x9 prototype matrix",
        "mismatch_reason": "Two WS2812/WS2811-family breakouts are not the belly-matrix design candidate.",
        "conflict_reason": "Explicit KEEP/REPLACE/UNDECIDED reconciliation is required; no silent substitution.",
    },
    "HW-010": {
        "electrical_blocker": "System power blocked pending input range, I/O identity, polarity, isolation, protection, and safe-current verification.",
        "protection_dependency": "Do not energize the Zi-E system from this board before reviewed bench verification.",
    },
    "HW-013": {"mechanical_blocker": "Verify connector pitch, mating, and orientation."},
    "HW-016": {"implementation_blocker": "Verify USB data capability; receipt/photo evidence does not prove data conductors."},
    "HW-022": {"electrical_blocker": "Verify switch continuity, common/contact pins, and rating."},
    "HW-023": {"electrical_blocker": "Verify switch continuity, common/contact pins, and rating."},
    "HW-024": {"mechanical_blocker": "Verify contact family, pitch, mating, orientation, crimp tool, and pull test."},
    "HW-025": {"mechanical_blocker": "Verify connector pitch, mating, contact latch, and orientation."},
    "HW-026": {"mechanical_blocker": "Verify connector pitch, mating, header/housing identity, and orientation."},
    "HW-027": {"mechanical_blocker": "Verify connector pitch, mating, header/housing identity, and orientation."},
    "HW-028": {"mechanical_blocker": "Verify connector pitch, mating, header/housing identity, and orientation."},
    "HW-029": {"mechanical_blocker": "Verify connector pitch, rating, mating, and orientation."},
    "HW-030": {"mechanical_blocker": "Verify connector pitch, rating, mating, and orientation."},
}


def _column_index(reference: str) -> int:
    letters = re.match(r"[A-Z]+", reference)
    if not letters:
        raise ValueError(f"invalid cell reference: {reference}")
    result = 0
    for letter in letters.group():
        result = result * 26 + ord(letter) - ord("A") + 1
    return result - 1


def _read_rows(source: Path, sheet_name: str) -> list[list[str]]:
    main = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
    office_rel = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
    with zipfile.ZipFile(source) as archive:
        shared: list[str] = []
        if "xl/sharedStrings.xml" in archive.namelist():
            root = ET.fromstring(archive.read("xl/sharedStrings.xml"))
            shared = ["".join(node.text or "" for node in item.iter(f"{{{main}}}t")) for item in root]
        workbook = ET.fromstring(archive.read("xl/workbook.xml"))
        relationships = ET.fromstring(archive.read("xl/_rels/workbook.xml.rels"))
        targets = {node.attrib["Id"]: node.attrib["Target"] for node in relationships}
        sheet = next((node for node in workbook.find(f"{{{main}}}sheets") or ()
                      if node.attrib.get("name") == sheet_name), None)
        if sheet is None:
            raise ValueError(f"required sheet missing: {sheet_name}")
        target = targets[sheet.attrib[f"{{{office_rel}}}id"]].lstrip("/")
        target = target if target.startswith("xl/") else f"xl/{target}"
        root = ET.fromstring(archive.read(target))
        rows: list[list[str]] = []
        for source_row in root.findall(f".//{{{main}}}sheetData/{{{main}}}row"):
            values: dict[int, str] = {}
            for cell in source_row.findall(f"{{{main}}}c"):
                value_node = cell.find(f"{{{main}}}v")
                value = "" if value_node is None else value_node.text or ""
                if cell.attrib.get("t") == "s" and value:
                    value = shared[int(value)]
                values[_column_index(cell.attrib["r"])] = value.strip()
            if values:
                rows.append([values.get(index, "") for index in range(max(values) + 1)])
        return rows


def _evidence_references(value: str) -> tuple[str, str]:
    fragments = [fragment.strip() for fragment in value.split(";") if fragment.strip()]
    photos = "; ".join(fragment for fragment in fragments
                       if "IMG_" in fragment or "Photo set" in fragment)
    orders = "; ".join(fragment for fragment in fragments
                       if "SKU" in fragment or "seller screenshot" in fragment.lower()
                       or "receipt" in fragment.lower())
    return photos, orders


def convert(source: Path) -> bytes:
    if source.name != SOURCE_FILENAME:
        raise ValueError(f"source filename must be {SOURCE_FILENAME}")
    rows = _read_rows(source, "Parts Inventory")
    header = next((row for row in rows if row and row[0] == "ID"), None)
    if header is None or header[:16] != ["ID", "Category", "Part", "Model / Marking", "Role", "Qty",
            "Unit", "Status / Evidence", "Confidence", "Reference specifications", "Interface",
            "Power / Voltage", "Zi-E use", "Photo / SKU evidence", "Software need",
            "Verification & safety notes"]:
        raise ValueError("Parts Inventory header/layout does not match the reviewed source")
    indexed = {row[0]: row for row in rows if row and re.fullmatch(r"HW-\d{3}", row[0])}
    expected = set(SUPPORTED_IDS + CANDIDATE_ONLY_IDS)
    if set(indexed) != expected:
        raise ValueError("workbook IDs must be exactly HW-001 through HW-033")
    columns = inventory_schema()
    output = io.StringIO(newline="")
    writer = csv.DictWriter(output, fieldnames=columns, lineterminator="\n")
    writer.writeheader()
    total = 0
    for inventory_id in SUPPORTED_IDS:
        row = indexed[inventory_id] + [""] * (16 - len(indexed[inventory_id]))
        status = row[7].casefold()
        if inventory_id == "HW-002":
            purchase_status, verify = "ordered", "yes"
        elif inventory_id == "HW-017":
            purchase_status, verify = "existing", "yes"
        elif "photo" in status:
            purchase_status, verify = "received", "yes"
        else:
            raise ValueError(f"unsupported evidence state for {inventory_id}: {row[7]}")
        quantity = int(row[5])
        total += quantity
        photo_reference, purchase_reference = _evidence_references(row[13])
        notes = " | ".join(filter(None, (
            f"Workbook category: {row[1]}", f"Workbook role: {row[4]}",
            f"Evidence wording: {row[7]}", f"Confidence wording: {row[8]}",
            f"Reference-family specifications (not measured): {row[9]}",
            f"Software need (not implementation proof): {row[14]}", row[15],
            "Receipt confirms purchase; physical receipt is unresolved because no photo is clearly matched."
                if inventory_id == "HW-017" else "",
            "Seller camera identity and pin maps are provisional procurement references only; arrival review is required before wiring or firmware pin freeze."
                if inventory_id == "HW-002" else "",
            "Received VL53L0X/GY-53/CJMCU-style hardware does not satisfy the VL53L1X head-rangefinder candidate without an explicit KEEP/REPLACE decision."
                if inventory_id == "HW-007" else "",
            "These two RGB breakouts do not satisfy the different belly-matrix design candidate without an explicit KEEP/REPLACE decision."
                if inventory_id == "HW-008" else "",
            "Electrically blocked for system power pending input range, terminals, polarity, isolation, protection, and safe-current verification."
                if inventory_id == "HW-010" else "",
        )))
        item = {column: "" for column in columns}
        item.update({
            "inventory_id": inventory_id, "part_name": row[2], "model_exact_variant": row[3],
            "quantity": quantity, "purchase_status": purchase_status, "procurement_role": "existing",
            "purchase_link_reference": purchase_reference, "photo_reference": photo_reference,
            "function": row[12], "interface_documented": row[10], "voltage_documented": row[11],
            "verify_on_arrival": verify, "decision_keep_replace_undecided": "UNDECIDED",
            "notes": notes, "evidence_source": f"{SOURCE_FILENAME} | Parts Inventory | {inventory_id}",
        })
        writer.writerow(item)
    if total != EXPECTED_QUANTITY:
        raise ValueError(f"supported quantity must total {EXPECTED_QUANTITY}, got {total}")
    return output.getvalue().encode("utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--inventory-root", type=Path)
    parser.add_argument("--confirm-import", action="store_true")
    parser.add_argument("--expected-revision", type=int, default=0)
    args = parser.parse_args()
    content = convert(args.source.resolve())
    store = InventoryStore(args.inventory_root.resolve(), inventory_schema()) if args.inventory_root else None
    preview = store.preview_csv(content) if store else None
    if preview and preview["count"] != 31:
        raise ValueError("canonical store preview did not contain exactly 31 rows")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(content)
    result = None
    if args.confirm_import:
        if store is None:
            raise ValueError("--inventory-root is required with --confirm-import")
        result = store.import_csv(content, args.expected_revision)
        revision = result["revision"]
        for inventory_id, annotations in BOUNDARY_ANNOTATIONS.items():
            result = store.review(inventory_id, annotations, revision)
            revision = result["revision"]
    digest = hashlib.sha256(args.source.read_bytes()).hexdigest()
    print(f"source={args.source.resolve()}")
    print(f"source_sha256={digest}")
    print("active_rows=31 total_quantity=167 excluded_candidates=HW-032,HW-033")
    print(f"output={args.output.resolve()}")
    print(f"persisted={bool(result)} robot_authority=NONE")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
