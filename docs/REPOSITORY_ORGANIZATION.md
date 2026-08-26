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

Nothing historical is moved or deleted by this refactor. Existing root bundles,
snapshots, old CAD exports, and generated packages should eventually be inventoried
before relocation. After provenance and references are checked, historical root files
should move into a dated, read-only area under `legacy/` or the applicable archive
directory (for example `mechanical/cad/archive/`). This is an organization target, not
authorization to move them.

Files under `legacy/` remain untrusted reference input. They must not be imported into
active firmware or electronics without primary-source verification, license/provenance
review where applicable, and conflict review against current decisions.
