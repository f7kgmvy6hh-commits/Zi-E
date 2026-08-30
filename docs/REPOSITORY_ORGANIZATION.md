# Repository Organization

## Active source-of-truth areas

- `docs/`: approved specifications, decisions, current state, audits, and session archive.
- `firmware/`: embedded interfaces, services, drivers, profiles, and host tests.
- `electronics/`: future verified schematics, PCB, power, wiring, and bring-up records.
- `software/`: laptop/future-phone AI brain and behavior orchestration.
- `tests/`: repository-level integration, simulation, and hardware bring-up tests.
- `mechanical/cad/current/`: current mechanical source and generated deliverables.

## Firmware organization

The detailed firmware layout is in `firmware/README.md`. Shared interfaces remain
vendor-neutral. Driver directories are separated by owning MCU. Profile composition is
the only place that assembles concrete hardware into robot services.

## Historical material

The 2026-08-30 maintenance pass inventoried and relocated the known root Git bundles,
the Zi-E Lite v0.1 export, and the grouped v0.3 migration export/manifests under
`legacy/snapshots/` and `legacy/exports/`. Each destination has a README recording
its original path, classification, and non-authoritative status. The former
chronological Current State was preserved under `docs/archive/status/`; the active
`docs/CURRENT_STATE.md` is concise current truth.

Use this flow for future candidates: inspect purpose, search references, preserve
manifest/checksum relationships, classify, then perform a same-repository move and
repair local references. Never delete uncertain history. Moving an artifact in the
current tree does not reduce prior Git history; history rewriting is a separate,
destructive operation requiring explicit authorization.

Files under `legacy/` remain untrusted reference input. They must not be imported into
active firmware or electronics without primary-source verification, license/provenance
review where applicable, and conflict review against current decisions.

Root README is the human landing page, REPOSITORY_MAP maps paths, PROJECT_CONTEXT
holds stable context, and START_HERE migration artifacts belong with the export they
describe rather than at root.
