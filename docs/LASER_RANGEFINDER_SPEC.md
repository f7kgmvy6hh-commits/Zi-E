# ZI-E v0.3 — Dedicated Laser Rangefinder Specification

## Decision
ZI-E v0.3 adds one dedicated forward laser Time-of-Flight (ToF) rangefinder in the head sensor brow. This is **not a visible laser pointer**. The primary candidate is the ST VL53L1X, which uses an invisible 940 nm Class 1 laser emitter.

## Why add it when ZI-E already has VL53L4CD sensors?
The lower-body VL53L4CD sensors are short-range safety/proximity devices and are positioned for cliff/collision coverage. The head rangefinder has a different role:
- exact forward distance to the point ZI-E is looking at,
- camera/object-distance assistance,
- approach-to-object measurement,
- measuring a wall/person/object on request,
- calibration data for vision and manipulation,
- a redundant non-safety source for high-level navigation/interaction.

It is **not** the final cliff/collision safety barrier. STM32 downward sensors + bumper + local safety rules remain authoritative.

## Candidate set
### A — VL53L1X — PRIMARY
- Range: up to 4 m.
- Update rate: up to 50 Hz.
- Optical source: invisible 940 nm, Class 1.
- Bare package: about 4.9 × 2.5 × 1.56 mm.
- Prototype breakout envelope reserved in CAD: 25.5 × 17.5 × 4.6 mm.
- Interface: I2C up to 400 kHz, interrupt + XSHUT available.
- Programmable Region of Interest (ROI) can narrow the effective field of view.
- Reason: enough range for a desktop companion, extremely small final package, cheaper/simpler packaging than larger LiDAR modules, same ST ToF ecosystem already used elsewhere.

### B — Benewake TF-Luna — SWITCH IF >4 m OR HIGH RATE IS REQUIRED
- 0.2–8 m.
- 1–250 Hz.
- 850 nm Class 1 VCSEL.
- 35 × 21.25 × 13.5 mm, <5 g.
- UART / I2C / I/O.
- Typical prototype price class observed around US$21–25.
- Switch condition: field tests show 4 m is insufficient, or 50 Hz is insufficient for the intended behavior.

### C — Benewake TFmini-S — LONGER-RANGE FALLBACK
- Up to ~12 m class.
- Larger and higher-power than VL53L1X.
- Only justified if real requirements prove a need beyond TF-Luna/VL53L1X.

## Mechanical integration
- OV5640 camera stays top-center.
- VL53L1X sits to the camera's right on the modular sensor-brow PCB; microphones remain symmetric around the brow.
- CAD reserves the full prototype breakout envelope so the first bench build can use a documented carrier.
- Final custom brow PCB should use the bare sensor or a much smaller carrier.
- Sensor and camera axes must be calibrated. A laser reading is a **single-point** range; software must not assume it measures every object visible in the camera image.

## Optical-window rule — important
ST documentation shows that cover windows can create ToF crosstalk. The brow therefore gets a dedicated IR-qualified optical region rather than simply placing the sensor behind arbitrary smoked plastic.

Design rules for the VL53L1X optical area:
- high transmission around 930–950 nm; ST recommends >85% for the material,
- keep the air gap between sensor and cover window as small as practical,
- keep window/sensor angle small (target <10°),
- prevent internal light leakage between emitter and receiver using black optical baffling/gasket,
- avoid haze/fingerprints/dust in the optical exclusion region,
- run offset + crosstalk calibration after the final window is installed,
- treat window material/paint/coating as a test item, not an aesthetic assumption.

## Electrical integration
Prototype path:
`VL53L1X -> I2C -> ESP32-S3 multimedia controller`

Recommended signals:
- 2.8/3.3 V-compatible local carrier supply depending on implementation,
- SDA/SCL,
- XSHUT/reset control,
- optional interrupt GPIO.

The lower-body VL53L4CD safety sensors remain on the STM32 safety domain, so a crash/reboot of the ESP32/head rangefinder cannot remove cliff protection.

## Software behavior
High-level API examples:
- `zie.measure_distance()`
- `zie.look_and_measure()`
- `zie.approach_until(distance_mm)` — still subject to STM32 collision/cliff limits
- camera overlay can show the distance at the calibrated ranging point.

Range data must include timestamp + validity/status. Stale or invalid data is never reused as a current measurement.

## Pre-purchase tests
Before final sensor/window/PCB freeze:
1. 50 mm, 100 mm, 300 mm, 1 m, 2 m, 4 m targets.
2. White / matte black / glossy / metal / glass targets.
3. Daylight and dark conditions.
4. Final candidate smoked/IR window coupons.
5. Dust/fingerprint sensitivity.
6. Pan/Tilt motion while ranging.
7. Camera-to-rangefinder extrinsic alignment calibration.
8. Compare VL53L1X against a ruler/reference distance.
9. Verify no harmful interaction with the lower ToF sensors in real robot geometry.
10. Record error distribution; do not rely on a single sample.

## Sources checked for v0.3
- ST VL53L1X product page and datasheet: https://www.st.com/en/imaging-and-photonics-solutions/vl53l1x.html
- ST VL53L1X datasheet: https://www.st.com/resource/en/datasheet/vl53l1x.pdf
- ST AN5231 cover-window guidelines: https://www.st.com/resource/en/application_note/dm00542648-cover-window-guidelines-for-the-vl53l1x-long-distance-ranging-time-of-flight-sensor-stmicroelectronics.pdf
- Adafruit VL53L1X carrier reference: https://www.adafruit.com/product/3967
- Benewake TF-Luna official: https://en.benewake.com/TFLuna/index.html?proid=328
- Benewake TF-Luna datasheet: https://en.benewake.com/uploadfiles/2024/04/20240426135921367.pdf
