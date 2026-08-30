from pathlib import Path

import pytest

from app.presence.face_engine import FacePack, FaceState, FaceVariant, validate_face_pack
from app.presence.face_library import DeviceFaceCache, FaceLibrary
from tools.facepacks.validate import load_manifest


CORE = Path("app/presence/core_face_pack.json")


def test_core_pack_validates_and_indexes_without_loading_assets():
    pack = load_manifest(CORE)
    library = FaceLibrary([pack])
    assert library.search(state=FaceState.LISTENING)[0].pack.pack_id == "zie-core-procedural"
    assert library.search(tag="core", offline_only=True)


def test_library_and_device_cache_are_bounded_and_duplicate_safe():
    pack = load_manifest(CORE)
    library = FaceLibrary([pack])
    with pytest.raises(ValueError, match="duplicate"):
        library.add(pack)
    cache = DeviceFaceCache(1)
    cache.enable(pack)
    cache.enable(pack)
    assert cache.planned_pack_ids == (pack.pack_id,)


@pytest.mark.parametrize("changes", [
    {"content_hash": "bad"}, {"license_id": ""}, {"provenance": ""},
    {"storage_class": "remote"}, {"engine_compatibility": "face-engine-v99"},
    {"tags": tuple(f"tag-{i}" for i in range(33))}, {"estimated_storage_bytes": 99_000_000},
])
def test_pack_metadata_fail_closed(changes):
    pack = load_manifest(CORE)
    values = dict(pack.__dict__)
    values.update(changes)
    with pytest.raises(ValueError):
        validate_face_pack(FacePack(**values))


def test_online_and_offline_disallowed_variants_fall_back_to_idle():
    pack = load_manifest(CORE)
    blocked = FaceVariant("blocked", FaceState.ALERT, ("asset.procedural/blocked",),
                          online_allowed=False, offline_allowed=False)
    values = dict(pack.__dict__)
    values["variants"] = pack.variants + (blocked,)
    validate_face_pack(FacePack(**values))
