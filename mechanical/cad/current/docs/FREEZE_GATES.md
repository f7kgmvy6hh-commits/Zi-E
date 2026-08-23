# ZI-E CAD v0.3 — Physical Freeze Gates

The parametric prototype CAD is complete. Production freeze requires the following measured evidence.

1. Display: exact FPC pinout, touch-controller IC, connector drawing, real thickness/adhesive stack.
2. Head: measured assembled mass and center of gravity; Tilt torque/temperature/jitter test.
3. Head lift: jam, obstruction reverse, power-loss descent, counterbalance, 1,000+ cycle cable/lift rig.
4. Arms: measured link/tool masses; shoulder/elbow horizontal-hold thermal test; backlash/collision/stow cycle test.
5. Telescope: real lead-screw/nut backlash, end sensors, thrust bearing, cable-flex endurance.
6. Wrist/tool: quick-swap insertion cycle, retention torque, contact current/temperature, mis-key/hot-plug tests.
7. Magnet: pull-force vs material/air gap and continuous/intermittent thermal test.
8. Wheels: tread traction, braking distance, turn scrub, fold-lock hard-stop load, encoder calibration.
9. Stability: tilt-table and dynamic braking with head high/low, one arm extended, dual-arm restricted cases.
10. Cliff sensing: white, matte black, glossy black, metal, mirror, clear glass, cloth, rounded edge, sunlight, dirt/occlusion.
11. Battery: measured system current, BMS shunt/FET/fuse sizing, connector temperature, cassette impact/service test.
12. Thermal: worst normal load + protected shutdown thermal soak with final shell.
13. RF/EMI: Wi-Fi/BLE performance with shell, motor noise, speaker, magnet tool and high-current wiring installed.
14. Printing/fit: printer/material calibration coupons before freezing 0.3/0.6 mm nominal fit clearances.
15. Service: repeated rear-cover/battery/tool/wheel module removal with no cable damage and no loss of alignment.

Any failed gate returns the affected parameters to CONDITIONAL A/B/C status with documented switch conditions.


## Hidden Belly Matrix freeze gate
Do not freeze custom LED PCB or optical insert until dead-front coupon tests prove off-state concealment, icon/clock readability, daylight performance, light-bleed control, current and temperature.

## Commissioning / controls freeze gate
No normal automated motion until actuator identity, model, voltage, direction, zero/range and communication health have passed low-torque commissioning. Motion protocol must distinguish received/accepted from actually executed.


## Laser rangefinder / sensor brow
- Validate prototype carrier fit and final bare-sensor plan.
- Verify IR cover transmission/crosstalk with real material coupons.
- Calibrate camera-to-rangefinder alignment.
- Complete known-distance and target-material tests before optics are frozen.
