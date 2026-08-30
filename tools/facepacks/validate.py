from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
if str(REPOSITORY_ROOT) not in sys.path:
    sys.path.insert(0, str(REPOSITORY_ROOT))

from app.presence.face_engine import FacePack, FaceState, FaceVariant, validate_face_pack


MAX_MANIFEST_BYTES = 256 * 1024


def load_manifest(path: Path) -> FacePack:
    resolved = path.resolve(strict=True)
    if resolved.suffix.lower() != ".json" or resolved.stat().st_size > MAX_MANIFEST_BYTES:
        raise ValueError("manifest must be a bounded JSON file")
    raw = json.loads(resolved.read_text(encoding="utf-8"))
    if not isinstance(raw, dict):
        raise ValueError("manifest root must be an object")
    allowed = {
        "pack_id", "display_name", "version", "engine_compatibility", "author",
        "license_id", "provenance", "content_hash", "tags", "style",
        "storage_class", "estimated_storage_bytes", "minimum_engine_version", "variants",
    }
    if set(raw) - allowed:
        raise ValueError("unknown manifest fields")
    variants = tuple(
        FaceVariant(
            item["variant_id"], FaceState(item["state"]), tuple(item["asset_refs"]),
            item.get("weight", 1), item.get("online_allowed", True),
            item.get("offline_allowed", True),
        ) for item in raw.get("variants", [])
    )
    pack = FacePack(
        raw["pack_id"], raw["version"], raw["engine_compatibility"], raw["content_hash"],
        raw["license_id"], raw["provenance"], raw["storage_class"], variants,
        raw.get("display_name", ""), raw.get("author", ""), tuple(raw.get("tags", [])),
        raw.get("style", ""), raw.get("estimated_storage_bytes", 0),
        raw.get("minimum_engine_version", "1.0"),
    )
    validate_face_pack(pack)
    return pack


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate one bounded Zi-E Face Pack manifest")
    parser.add_argument("manifest", type=Path)
    args = parser.parse_args()
    try:
        pack = load_manifest(args.manifest)
    except (OSError, KeyError, TypeError, ValueError, json.JSONDecodeError) as exc:
        print(f"INVALID: {exc}", file=sys.stderr)
        return 1
    print(json.dumps({"status": "VALID", "pack_id": pack.pack_id,
                      "variants": len(pack.variants),
                      "estimated_storage_bytes": pack.estimated_storage_bytes}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
