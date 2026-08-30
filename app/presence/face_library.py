from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable

from .face_engine import FacePack, FaceState, validate_face_pack


MAX_LIBRARY_PACKS = 2048
MAX_DEVICE_CACHE_PACKS = 8


@dataclass(frozen=True)
class FacePackRecord:
    pack: FacePack
    installed_on_device: bool = False
    validated: bool = True


class FaceLibrary:
    """Metadata-only host catalogue; asset bytes stay outside the index."""

    def __init__(self, packs: Iterable[FacePack] = ()):
        self._records: dict[str, FacePackRecord] = {}
        for pack in packs:
            self.add(pack)

    def add(self, pack: FacePack) -> None:
        validate_face_pack(pack)
        if pack.pack_id in self._records:
            raise ValueError("duplicate pack id")
        if len(self._records) >= MAX_LIBRARY_PACKS:
            raise ValueError("face library capacity reached")
        self._records[pack.pack_id] = FacePackRecord(pack)

    def search(
        self, *, text: str = "", state: FaceState | None = None,
        tag: str | None = None, offline_only: bool = False,
        maximum_results: int = 100,
    ) -> tuple[FacePackRecord, ...]:
        if not 1 <= maximum_results <= 100:
            raise ValueError("invalid result bound")
        needle = text.casefold()[:96]
        results: list[FacePackRecord] = []
        for record in self._records.values():
            pack = record.pack
            if needle and needle not in f"{pack.pack_id} {pack.display_name} {pack.author} {pack.style}".casefold():
                continue
            if tag and tag not in pack.tags:
                continue
            if state and not any(item.state is state for item in pack.variants):
                continue
            if offline_only and not any(item.offline_allowed for item in pack.variants):
                continue
            results.append(record)
            if len(results) == maximum_results:
                break
        return tuple(results)


class DeviceFaceCache:
    """Plans validated pack identities for deployment; it transfers no files."""

    def __init__(self, maximum_packs: int = MAX_DEVICE_CACHE_PACKS):
        if not 1 <= maximum_packs <= MAX_DEVICE_CACHE_PACKS:
            raise ValueError("invalid device cache bound")
        self.maximum_packs = maximum_packs
        self._pack_ids: list[str] = []

    def enable(self, pack: FacePack) -> None:
        validate_face_pack(pack)
        if pack.pack_id in self._pack_ids:
            return
        if len(self._pack_ids) >= self.maximum_packs:
            raise ValueError("device face cache capacity reached")
        self._pack_ids.append(pack.pack_id)

    @property
    def planned_pack_ids(self) -> tuple[str, ...]:
        return tuple(self._pack_ids)
