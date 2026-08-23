# ZI-E CAD v1.1 — Prototype Engineering Release

This repository folder is the current source-of-truth CAD package for the ZI-E desktop companion robot.

## What this release is
- Complete parametric prototype CAD for packaging, structure, mechanisms, state envelopes, service access, tools, and safety keep-outs.
- Generated system configurations: ACTIVE, SLEEP, PROTECTED SHUTDOWN, and CUTAWAY.
- Individual STEP/STL exports for the main printable/mechanical parts.
- Automated engineering validation, requirements traceability, FMEA, mass/COM estimate, and component-envelope data.

## What this release is not
It is **not production-frozen**. Physical measurements and bench tests still govern final fits, torque, thermal behavior, cliff detection, cable fatigue, battery current limits, and final cosmetic surfacing.

## Authoritative files
1. `src/zie_cad.py` — master parametric CAD source.
2. `src/validate_design.py` — automated geometry/engineering checks.
3. `data/parameters.json` — generated parameter snapshot.
4. `data/component_envelopes.csv` — current component envelopes and source authority.
5. `data/requirements_traceability.csv` — requirements and verification path.
6. `docs/CAD_RELEASE.md` — release meaning and remaining freeze gates.
7. `docs/AI_CAD_WORKFLOW.md` — rules for future AI/CAD edits.

Generated STEP/STL/glTF files are outputs, not the editing source of truth.

## Current architecture preserved
- Exactly 2 powered wheels + hidden passive caster; differential drive.
- Wide landscape digital face; OV5640 AF camera; dual MEMS mic reservations.
- Head Pan + Tilt + vertical retract with Active/Sleep/Protected Shutdown states.
- Two functional arms: 2-DOF shoulder, elbow, one-stage telescope, limited roll, wrist pitch, quick-swap tool.
- Gripper and magnetic pickup tool architecture.
- Low central 3S2P battery cassette and rear service electronics.
- Four corner downward cliff reservations, horizontal proximity reservations, bumper fallback, anti-tip skids.
- ESP32-S3 multimedia/communications MCU separated from STM32 safety/motion controller.

## Build / regenerate
Requirements: Python 3, CadQuery 2.x, VTK for render helper.

```bash
cd ZI-E_CAD_v1_1
python src/zie_cad.py
python src/validate_design.py
```

Rendering example:

```bash
python src/render_gltf.py cad/ZI-E_v1_1_ACTIVE.gltf renders/ZI-E_v1_1_ACTIVE_iso.png 35 18
```

## Engineering rule
Every meaningful CAD change must run:

**Memory → Requirements → Sources → Alternatives → Interfaces → Parameters → Regenerate → Collision → Mass/COM → Stability → Power/Thermal → Cable sweep → DFM/Tolerance → Safety/FMEA → Verification → Change impact.**

See `docs/AI_CAD_WORKFLOW.md` and `docs/ENGINEERING_LOOPS.md`.

## v1.1 delta
- Added hidden **Belly Light Matrix (BLM)** secret-until-lit optical window, matrix keepout, baffle and prototype board envelope.
- Added pre-purchase open-source failure-harvest audit based on Reachy Mini, LeRobot SO-101, OpenArm, Stretch, MoveIt/Nav2 and MuJoCo lessons.
- Added commissioning, command-arbitration, dynamic safety-envelope and diagnostic requirements before bulk component purchase.
- Automated validation now runs 37 checks: 36 pass; the sole restricted case remains both arms forward with 2×250 g payload during normal driving.
