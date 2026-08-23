# Current CAD Status

The current CAD source of truth is `mechanical/cad/current/`.

## Snapshot
- Parametric source: `src/zie_cad.py`
- Validation: `src/validate_design.py`
- Rendering/export helper: `src/render_gltf.py`
- Four assembly states: ACTIVE, SLEEP, SHUTDOWN, CUTAWAY
- Individual STEP/STL exports for major structural/cosmetic parts
- Requirements traceability, FMEA, parameter data, bounding boxes, mass/COM, validation matrix and renders included.

## Automated validation snapshot
From `data/validation_summary.json`:
- 34 tests total
- 33 pass
- 1 restricted/verify case
- estimated mass ≈ 2422 g
- estimated active COM ≈ (-0.7, -1.8, 90.7) mm in CAD coordinates
- ACTIVE bounding box ≈ 248 mm × 173.86 mm × 282 mm
- SLEEP height ≈ 247 mm
- SHUTDOWN height ≈ 182 mm

The restricted case is both arms simultaneously carrying 250 g forward: static support margin is ~12.2 mm versus the 15 mm normal-driving target. Firmware therefore treats that combination as a restricted manipulation condition (base locked/very-low acceleration or lower payload).

## Interpretation
This is prototype engineering CAD, not production manufacturing freeze. Any low/medium-confidence mass, clearance, torque, acoustic, thermal, traction, magnetic-force or optical-edge assumption must be replaced by measured hardware data before release to manufacturing.
