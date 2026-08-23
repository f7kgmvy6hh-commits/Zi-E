# ZI-E

**Project Release v0.3** — Desktop AI Companion Robot

Includes the dedicated head laser ToF rangefinder and current CAD/pre-purchase design state.

ZI-E is an original, modular desktop AI robot combining an expressive digital face, camera/audio interaction, two functional arms with quick-swap tools, two-wheel differential mobility, local safety control, and an external laptop/future-phone AI brain.

## Current status
**Prototype component architecture + parametric CAD are complete enough to enter physical validation.** Production dimensions/parts are not frozen until the listed freeze-gate tests pass.

### Start here
- [`docs/ZI-E_MASTER_SPEC.md`](docs/ZI-E_MASTER_SPEC.md) — complete current architecture
- [`docs/COMPONENT_MASTER_MATRIX.md`](docs/COMPONENT_MASTER_MATRIX.md) — primary/backups/switch conditions
- [`docs/CURRENT_STATE.md`](docs/CURRENT_STATE.md) — project status
- [`docs/ENGINEERING_METHOD_AND_MEMORY_LOOP.md`](docs/ENGINEERING_METHOD_AND_MEMORY_LOOP.md) — mandatory design/review process
- [`mechanical/cad/current/`](mechanical/cad/current/) — current CAD source, STEP/STL, renders and validation data
- [`docs/OPEN_ITEMS_AND_FREEZE_GATES.md`](docs/OPEN_ITEMS_AND_FREEZE_GATES.md) — what must still be physically verified
- [`docs/GITHUB_HANDOFF_CURRENT.md`](docs/GITHUB_HANDOFF_CURRENT.md) — migration/upload guide

## Key architecture
- AI brain: laptop initially; phone may later become portable/edge brain.
- Multimedia/UI: ESP32-S3-WROOM-1-N16R8.
- Real-time safety/motion: STM32G0B1RET6.
- Face: 3.5-inch landscape IPS touch panel class + OV5640 AF camera.
- Head: Pan + Tilt + vertical retract with Active/Sleep/Protected Shutdown.
- Arms: 2-DOF shoulder, elbow, telescopic forearm, roll, wrist pitch, quick-swap tool.
- Tools: adaptive gripper and removable electromagnet tool.
- Mobility: exactly **two powered wheels + one passive ball caster**; no four-wheel/mecanum base in V1.
- Safety: local cliff sensors, bumper, IMU, current/jam supervision, hardware Motion Kill, state interlocks and safe-stop behavior.
- Battery: low-central 3S2P 18650 Li-ion pack architecture.

## Important warning
`legacy/` contains old WALL-E-era material with known contradictions and is **not source of truth**. Likewise, `assets/deprecated/` contains generated images that may show incorrect geometry. Use current docs/CAD instead.


## Latest design delta
CAD v0.3 includes the hidden **Belly Light Matrix** (secret-until-lit RGB information/ambient panel) and integrates a pre-purchase Open-Source Harvest audit. See `docs/BELLY_MATRIX_SPEC.md`, `docs/OPEN_SOURCE_HARVEST_AUDIT.md`, and `docs/PRE_PURCHASE_GATE.md`.
