# ZI-E AI-CAD Workflow — v1.0

This file defines how an AI agent must modify ZI-E CAD. It is intentionally stricter than “generate a shape that looks right.” The CAD is treated as an engineering model tied to requirements, components, tests, and revision history.

## 1. Master method: top-down parametric CAD

The authoritative source is the parameterized CadQuery model under `src/zie_cad.py`. STEP/STL/glTF files are generated outputs, not the source of truth.

Why:
- CadQuery supports scriptable parametric geometry, assemblies, constraints, and STEP/STL/glTF export: https://cadquery.readthedocs.io/en/latest/assy.html and https://cadquery.readthedocs.io/en/latest/classreference.html
- OpenSCAD likewise emphasizes parameterization/modules for programmable CAD: https://openscad.org/documentation.html
- Onshape’s parametric guidance recommends driving many parts from shared layout dimensions, keeping key driving dimensions early, using simple fully-defined sketches, and adding fillets late: https://www.onshape.com/en/resource-center/innovation-blog/how-onshape-has-fundamentally-improved-parametric-cad

For ZI-E, any important dimension must therefore be a named parameter or derived from named component envelopes. Do not scatter unexplained numeric “magic numbers” through the model.

## 2. Source authority ladder

When an AI needs a physical dimension or rating, use this order:
1. Manufacturer mechanical drawing / datasheet.
2. Manufacturer product page.
3. Reputable distributor product page.
4. Measured real part.
5. Only then: an explicit engineering estimate / keep-out envelope.

Never silently convert an estimate into a “real” dimension. `component_envelopes.csv` records authority/notes.

## 3. Requirements before geometry

Before changing geometry, identify the requirements affected and how the new version will be verified. NASA’s systems-engineering guidance recommends unique requirement IDs and a verification method such as test, inspection, analysis, or demonstration, with bidirectional traceability: https://www.nasa.gov/reference/system-engineering-handbook-appendix/

ZI-E applies this through `requirements_traceability.csv` and `validation_matrix.csv`.

## 4. Loads, constraints, keep-outs first

Autodesk’s generative-design documentation treats real-world constraints, loads, attached point masses, and manufacturing restrictions as inputs before shape generation. ZI-E follows the same principle: define preserve geometry and keep-out/swept volumes before beautifying the shell.

Sources:
- Design conditions / real-world loads: https://help.autodesk.com/view/fusion360/ENU/?guid=GD-DESIGN-CONDITIONS
- Manufacturing constraints: https://help.autodesk.com/cloudhelp/ENU/Fusion-GenerativeDesign/files/GD-SPECIFY-MFG-METHOD.htm

Examples in ZI-E:
- Head Bay is a protected swept volume.
- Battery extraction path is a keep-out.
- Arm motion envelope is a keep-out.
- Antenna keep-out excludes metal/high-current structures.
- Cable bend/sweep volumes are geometry, not afterthoughts.

## 5. Manufacturing-aware clearances

There is no universal printable tolerance. Prusa explicitly notes that fit depends on printer, orientation, geometry, calibration, settings and material, with at least ~0.3 mm a useful starting point for moving parts: https://help.prusa3d.com/article/modeling-with-3d-printing-in-mind_164135

Formlabs reports typical minimum clearances around 0.4–0.5 mm for its resin processes: https://formlabs.com/de/support/Design-specifications-for-3D-models-Form-4-generation/

Because ZI-E’s initial prototype is expected to use FDM for many structural/cosmetic parts, the CAD uses conservative starting values:
- moving interface: 0.60 mm nominal;
- static fit: 0.30 mm nominal;
- service seam: 0.50 mm nominal;
- cosmetic shell wall: 2.4 mm;
- structural printed wall/rib target: 3.2 mm.

These are **prototype parameters**, not universal tolerances. A printer/material calibration coupon must precede final fit freeze.

## 6. Configurations are states, not separate projects

One parameter model generates:
- ACTIVE;
- SLEEP;
- PROTECTED SHUTDOWN;
- CUTAWAY.

Onshape’s configuration guidance similarly treats variants as controlled configurations rather than unrelated files: https://www.onshape.com/en/features/configurations

This prevents state-specific geometry from drifting out of sync.

## 7. Mandatory AI loop for every CAD change

Every meaningful CAD change must run this sequence:

**Memory → Requirements → Sources → Alternatives → Interface check → Parameter update → Regenerate → Collision check → Mass/COM → Stability → Power/thermal impact → Cable sweep → DFM/tolerance → Safety/FMEA → Verification method → Change log.**

If any critical item is unresolved, the change is not “frozen”; it is marked conditional with A/B/C alternatives and switch conditions.

## 8. No fake precision

An AI must never present a CAD dimension as production-final when it came from an approximate breakout-board size, a generic image, or an unverified vendor listing. Exact examples still gated in this release include the chosen display FPC/touch-controller details and measured printed-part fit.

## 9. Functional geometry before cosmetic surfacing

The old ZI-E visual identity remains the style target: compact, soft, premium, not industrial-humanoid. But the shell must follow the validated internal package. Cosmetic fillets/surfaces are permitted only after Head Bay, wheel fold, arm stow, battery service, sensor fields of view, thermal airflow, antenna keep-out and cable corridors remain valid.

## 10. Release rule

An AI may call a version “prototype CAD complete” when all subsystems have modeled envelopes/interfaces and automated checks run. It may call a version “production frozen” only after the physical verification gates in `CAD_RELEASE.md` are passed with measured hardware.
