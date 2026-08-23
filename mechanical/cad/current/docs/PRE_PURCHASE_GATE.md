# ZI-E Pre-Purchase Gate

Do not bulk-order parts merely because a CAD envelope fits. Buy the minimum parts needed to retire the highest-risk uncertainties first.

## Phase A — cheap evidence before bulk purchase
1. One 3.5-inch display candidate + confirm exact pinout/touch IC.
2. One OV5640 AF module.
3. One ESP32-S3 N16R8 development carrier and one STM32G0B1 test board/carrier.
4. One STS3215, one HL2915, one SC09, one N20 encoder motor.
5. One wheel motor + DRV8876 test channel.
6. One VL53L4CD breakout initially; surface tests before buying all 8 sensors.
7. One IS31FL3741 13×9 belly-matrix breakout + several optical-window coupons.
8. One P20/15 magnet sample (two if generic supplier quality is uncertain).

## Mandatory bench tests before batch order
- Servo ID/model/baud discovery and safe low-torque commissioning.
- Sustained rated-load temperature, backlash, repeatability, audible noise.
- Head cable loop through Pan/Tilt/Lift full travel and jam/power-loss behavior.
- Arm dummy-load test using estimated link/tool masses.
- Wheel traction/braking/current on representative desk surfaces.
- Cliff sensor surface matrix: white, matte black, glossy black, wood, metal, mirror, clear glass, tablecloth, edge shapes, daylight, dirty sensor.
- Belly matrix optical coupon: hidden off-state, time/icon readability, daylight, current, heat, light bleed.
- Magnet pull force vs steel size/thickness/paint/gap and coil temperature.
- Power transient/regen and brownout fault injection.

## Bulk purchase condition
A part becomes `BUY-BATCH` only after its test result is stored in `/tests/results/` and the component matrix status is changed from `CONDITIONAL` to `VERIFIED`.


## v0.3 laser rangefinder
Buy only one VL53L1X carrier/sample first. Verify range accuracy, cover-window optical performance, camera alignment and contamination sensitivity before custom brow PCB or bulk order.
