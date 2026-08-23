# ZI-E CAD v1.0 — Automated Geometry / Engineering Validation

**Result:** 33/34 checks pass. Failed checks are design restrictions or freeze gates, not silently ignored.

Estimated mass: **2.42 kg**; active estimated COM: **(-0.7, -1.8, 90.7) mm**.

| Check | Value | Target | Result |
|---|---:|---|---|
| Head bay side clearance per side | 3.5 | >= 1.5 mm | PASS |
| Head bay depth clearance per side | 4.0 | >= 2.0 mm | PASS |
| Display side bezel each side | 15.740000000000002 | >= 10 mm | PASS |
| Display vertical margin total | 26.740000000000002 | >= 20 mm | PASS |
| Sleep visible active-screen fraction | 0.847 | >= 0.75 | PASS |
| Shutdown head top flush | 0.0 | <= 1 mm above body top | PASS |
| ACTIVE minimum Z | -0.0 | >= -0.1 mm | PASS |
| SLEEP minimum Z | -0.0 | >= -0.1 mm | PASS |
| SHUTDOWN minimum Z | -0.0 | >= -0.1 mm | PASS |
| Shutdown overall height | 182.0 | ~182 mm | PASS |
| Active overall height | 282.0 | <= 290 mm | PASS |
| Arm distal collision with torso: active | 0.0 | 0 mm^3 | PASS |
| Arm distal collision with torso: stow | 0.0 | 0 mm^3 | PASS |
| Arm distal collision with torso: reach | 0.0 | 0 mm^3 | PASS |
| Arm distal collision with torso: desk | 0.0 | 0 mm^3 | PASS |
| Active tire desk contact | 0.0 | ~0 mm | PASS |
| Wheel tire/base collision active | 1.072 | < 2 mm^3 numerical | PASS |
| Stowed wheel ground clearance | 5.96 | >= 4 mm | PASS |
| Wheel tire/base collision stowed | 0.0 | < 2 mm^3 numerical | PASS |
| Head shell/torso collision ACTIVE | 0.0 | 0 mm^3 | PASS |
| Head shell/torso collision SLEEP | 0.0 | 0 mm^3 | PASS |
| Head shell/torso collision SHUTDOWN | 0.0 | 0 mm^3 | PASS |
| Head lift nominal linear speed | 6.67 | ~6.7 mm/s | PASS |
| Head protected travel time | 15.0 | <= 18 s | PASS |
| Rated drive speed | 0.251 | 0.20-0.30 m/s | PASS |
| Head tilt rated torque margin | 2.7 | >= 2.0x | PASS |
| Wrist pitch rated torque margin | 1.98 | >= 1.7x | PASS |
| Shoulder rated torque sanity margin | 1.68 | >= 1.5x | PASS |
| Elbow rated torque sanity margin | 1.74 | >= 1.5x | PASS |
| Estimated total mass | 2422 | informational | PASS |
| Baseline static support margin | 19.8 | >= 18 mm | PASS |
| One-arm 250g forward static margin | 20.0 | >= 15 mm | PASS |
| Two-arm 2x250g forward static margin | 12.2 | >= 15 mm for normal driving | RESTRICT / VERIFY |
| Protected worst commanded rail budget | 15.5 | < 20 A cell ceiling | PASS |

## Important interpretation

- The two-arm 2×250 g forward case is intentionally allowed to fail the normal-driving margin. Firmware must treat that combination as a restricted manipulation state (base stationary or much lower payload/acceleration).
- Torque calculations use **rated torque**, never stall torque, and include a simple belt-loss factor. Physical thermal/load tests remain the authority.
- Mass values tagged low/medium confidence must be replaced with measured values after parts are printed/bought.
- Collision tests cover the defined key poses. Continuous trajectory collision checking belongs in the simulation/firmware phase.