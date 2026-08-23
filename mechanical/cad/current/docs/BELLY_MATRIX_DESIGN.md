# ZI-E Hidden Belly Light Matrix (BLM) — v0.1

## Purpose
A hidden, low-resolution RGB information/light surface in the front belly. When off, it should visually disappear into the body. When on, it can show status, time, battery, timers, directional cues, AI state, music visualization, warnings, charging, tool state, or ambient color.

This is **not** a second TFT and is **not** safety-critical.

## Prototype electronics
- Prototype carrier: Adafruit IS31FL3741 13×9 RGB matrix, product 5201.
- 117 RGB pixels, 3 mm pitch, 51.3 × 39.0 × 4.6 mm, 5.8 g.
- I2C controller with per-channel PWM and global current control.
- 5 V preferred for best RGB headroom.
- Prototype board is development hardware only; the CAD reserves 80 × 54 × 12 mm for a future wider custom matrix.

## CAD integration
- Flush optical window: 74 × 46 × 1.2 mm nominal.
- Window is a separate optical insert in the front torso opening.
- Internal black baffle prevents light bleed into seams/electronics and improves symbol contrast.
- No visible bezel is intended in the final cosmetic shell.
- BLM mass estimate including board/window/baffle/harness: 22 g.
- BLM stays on the fixed torso, so it adds no head/arm moving mass.

## Optical stack
Outside → inside:
1. Scratch-resistant cosmetic outer surface / matching finish.
2. Thin translucent PC/PMMA secret-until-lit layer.
3. Optional tinted/semi-opaque printed/coated layer to hide inactive pixels.
4. Diffusing layer only as much as necessary for uniformity.
5. Black baffle/light-well.
6. RGB LED matrix PCB.

The final panel must be selected by **physical optical coupons**, not by CAD alone. Prototype 0.8–1.5 mm window thicknesses with several transmission/diffusion treatments.

## Electrical architecture
- Powered from 5 V multimedia/lighting rail.
- Controlled by ESP32-S3 on a **dedicated non-safety I2C bus** where practical.
- Do not share a bus dependency with cliff/safety sensing.
- Hardware enable/load switch allows BLM to be fully off in protected shutdown and after a fault.
- Firmware imposes a global brightness/current ceiling; 100% full-white is not a normal operating mode.
- Optional NTC on final custom PCB for thermal derating.

## Software behavior
The AI/daemon requests semantic effects, not raw unsafe power:
- `show_clock()`
- `show_battery()`
- `show_timer()`
- `ambient(color/pattern)`
- `thinking()`
- `charging()`
- `tool_active()`
- `warning()`

Safety status has priority over cosmetic/AI animations. A disabled or failed BLM must never stop core robot operation.

## Risks found before purchase
1. **I2C bandwidth:** IS31FL3741 is appropriate for simple low-resolution graphics but not video. Use buffered updates and up to ~1 MHz I2C on capable hardware; do not design video-like UI around it.
2. **Light bleed / blurry text:** baffle and optical distance must be tested with real parts.
3. **Invisible-when-off challenge:** an opaque white FDM shell cannot magically transmit RGB. Use a dedicated translucent optical insert with body-matched finish/coating.
4. **Sunlight readability:** brightness/transmission must be validated at desk daylight levels.
5. **Heat/current:** cap global brightness; leave rear air volume/copper and do not sandwich the PCB in insulating foam.
6. **Serviceability:** matrix carrier and optical insert must be removable from the rear service path without removing the structural torso.

## Freeze gate
Do not order a custom belly PCB or final optical panel until a cheap bench coupon proves:
- off-state concealment,
- RGB color quality,
- clock/icon readability,
- no objectionable hot spots,
- acceptable temperature/current,
- acceptable daylight visibility,
- no light leak into nearby seams/sensors.

## References reviewed 2026-08-23
- Adafruit IS31FL3741 guide: https://learn.adafruit.com/adafruit-is31fl3741
- Adafruit product 5201: https://www.adafruit.com/product/5201
- Hidden-until-lit panel construction example: https://patents.google.com/patent/EP4100939B1/en
- Covestro hidden-until-lit light-management example: https://solutions.covestro.com/en/highlights/articles/cases/2023/advanced-display-solutions-in-mobility
- ACRYLITE secret-until-lit transmission examples: https://www.acrylite-polymers.com/en/sample-box-secret-until-lit
