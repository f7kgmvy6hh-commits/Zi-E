# ZI-E Project Release v0.3 — 2026-08-23

## Added
- Dedicated head laser ToF rangefinder subsystem.
- Primary: VL53L1X; TF-Luna and TFmini-S are conditional backups.
- Head-brow CAD envelope for prototype VL53L1X carrier while preserving centered OV5640 camera.
- IR cover-window/crosstalk design rules and calibration gate.
- Camera/rangefinder alignment requirement and timestamp/validity semantics.
- New pre-purchase distance/optical-window tests.

## Preserved
- Exactly two powered differential-drive wheels + hidden passive ball caster.
- Hidden Belly Light Matrix / secret-until-lit panel.
- ESP32-S3 multimedia + STM32 safety/motion split.
- Pan/Tilt/Lift retracting head.
- Dual modular arms, quick-swap gripper/magnet tools.
- 3S2P battery architecture, safety domains, Memory Loop, Engineering Loop, Open-Source Harvest Loop.

## CAD status
- Current release name: v0.3.
- The prior v1.1 CAD snapshot is retained under `mechanical/cad/archive/v1.1/`; it is historical, not current source of truth.
- v0.3 automated validation: 39 checks, 38 pass, 1 restricted scenario.
- The restricted scenario remains two arms simultaneously forward with 250 g payload each during normal driving; firmware must lock/strongly derate the base.
- Estimated mass after rangefinder allowance: ~2.448 kg pending measured prototype masses.

## Purchase status
The new rangefinder is not `BUY-BATCH`. Buy one prototype sensor/carrier first and pass the tests in `LASER_RANGEFINDER_SPEC.md` and `PRE_PURCHASE_GATE.md`.
