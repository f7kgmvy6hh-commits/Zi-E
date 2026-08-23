# ZI-E CAD v0.3 — Web / Datasheet Research Basis

This file records the external engineering sources used to shape the CAD methodology and verify key component envelopes. Vendor dimensions remain secondary to measured real hardware once parts arrive.

## CAD methodology
- CadQuery documentation — programmable parametric solids, assemblies, constraints, STEP/STL/glTF export:
  https://cadquery.readthedocs.io/en/stable/
  https://cadquery.readthedocs.io/en/latest/assy.html
- OpenSCAD documentation — parameterization, reusable modules/functions, programmable CAD:
  https://openscad.org/documentation.html
- FreeCAD Python scripting tutorial — scriptable parametric CAD workflow:
  https://github.com/FreeCAD/FreeCAD-documentation/blob/main/wiki/Python_scripting_tutorial.md
- Onshape parametric CAD guidance — top-down layout dimensions, simple named features, configurations, late fillets:
  https://www.onshape.com/en/resource-center/innovation-blog/how-onshape-has-fundamentally-improved-parametric-cad
- NASA Systems Engineering Handbook appendix — unique requirement IDs, verification matrices, bidirectional traceability, analysis/inspection/test/demonstration:
  https://www.nasa.gov/reference/system-engineering-handbook-appendix/
- Prusa design-for-print guidance — no universal tolerance; movable interfaces typically need a starting clearance and printer/material calibration:
  https://help.prusa3d.com/article/modeling-with-3d-printing-in-mind_164135

## Key component envelope authority
- ESP32-S3-WROOM-1 family: 18 x 25.5 x 3.1 mm class; N16R8 = 16 MB Flash + 8 MB PSRAM:
  https://www.espressif.com/en/products/modules/esp32-s3/esp32-s3-wroom-1
- Molicel INR-18650-M35A: diameter max 18.6 mm, height max 65.2 mm, weight max 48 g, 3.6 V nominal, 3.5 Ah class, 10 A discharge:
  https://www.molicel.com/product/inr-18650-m35a/
- Feetech STS3215: 45.2 x 24.7 x 35 mm mechanical envelope:
  https://www.feetech.cn/Data/feetechrc/upload/file/20200611/6372749961523760249976542.pdf
- Waveshare SC09: 23.2 x 12.0 x 25.5 mm; serial feedback servo:
  https://www.waveshare.com/wiki/SC09_Servo
- ST VL53L4CD: bare module 4.4 x 2.4 x 1 mm; up to 100 Hz; 2.8 V supply. CAD reserves a larger carrier/optical-window envelope for prototyping:
  https://www.st.com/en/imaging-and-photonics-solutions/vl53l4cd.html
- PUI AS04004PO-R speaker: 40 x 28.4 x 13 mm, 4 ohm, 2 W rated:
  https://puiaudio.com/product/speakers-and-receivers/as04004po-r

## Source authority rule
Use, in order:
1. Manufacturer mechanical drawing/datasheet.
2. Manufacturer product page.
3. Reputable distributor.
4. Measured real part.
5. Explicitly labeled engineering envelope/estimate.

Never silently promote an estimate to an exact production dimension.


## v0.3 open-source failure-harvest sources
See `OPEN_SOURCE_HARVEST_AUDIT.md` for Reachy Mini, LeRobot SO-101, OpenArm, Stretch, MoveIt, Nav2, MuJoCo and Adafruit issue/doc references and the resulting preventive design changes.


## Head laser rangefinder — v0.3
- ST VL53L1X product/datasheet: https://www.st.com/en/imaging-and-photonics-solutions/vl53l1x.html
- ST VL53L1X datasheet: https://www.st.com/resource/en/datasheet/vl53l1x.pdf
- ST AN5231 cover-window/crosstalk guidance: https://www.st.com/resource/en/application_note/dm00542648-cover-window-guidelines-for-the-vl53l1x-long-distance-ranging-time-of-flight-sensor-stmicroelectronics.pdf
- Adafruit carrier reference: https://www.adafruit.com/product/3967
- Benewake TF-Luna alternative: https://en.benewake.com/TFLuna/index.html?proid=328
