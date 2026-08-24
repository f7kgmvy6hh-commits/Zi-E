# ZI-E Lite Aluminum v0.1 — MASTER ALL-IN-ONE

This file consolidates the complete Lite branch into one readable reference. The individual source files remain in the repository because CAD, wiring, templates and software notes are easier to maintain separately.

---

# ZI-E Lite v0.1 — Current State

This repository is the source of truth for the **Lite, stationary, fixed-head, aluminum-body** ZI-E prototype.

## Frozen for this branch
- No 3D-printed structural body.
- Aluminum shell/body fabricated from cut sheet.
- No arms, wheels, head Pan/Tilt/Lift motors, laser, cliff sensors, proximity array, magnetic tool, or motion MCU.
- Head is fixed/non-motorized; final visual integration can be refined after software bring-up.
- Main integrated electronics candidate: Waveshare ESP32-S3-Touch-LCD-3.5B.
- Camera: OV5640-compatible DVP module.
- Audio: onboard microphone/audio path + included speaker first.
- Emotion light: one hidden diffused RGB LED, not the large belly matrix.
- USB-C first. Battery is optional and purchased later after measured current/runtime.
- Software-first workflow: finish face/vision/audio/network/AI integration before buying the deferred mechanical parts.

## Not frozen until real parts are in hand
- Exact camera bracket and lens opening.
- Microphone acoustic duct and speaker vent geometry.
- Final USB-C extension/port geometry.
- Battery strap/holder spacing.
- Cosmetic surface finish.
- Exact RF-window implementation after Wi-Fi/BLE testing in the aluminum enclosure.

## Upgrade compatibility kept
The body keeps removable head/base interfaces and internal keep-outs so a future motorized head, mobile base, larger belly display and arms can be added without treating this Lite prototype as the final mechanical robot.


---

# ZI-E Lite Aluminum v0.1 - Master Specification

## 1. Goal
Build the cheapest useful physical ZI-E first: a stationary AI companion with face, touch, vision, voice, Wi-Fi/BLE, RTC/IMU, and one hidden RGB emotion light. Software is completed and validated before the expensive motion system is purchased.

## 2. Architecture change from the earlier discrete Lite plan
The engineering loop found a simpler integrated option: the Waveshare ESP32-S3-Touch-LCD-3.5B combines the 3.5-inch capacitive display, ESP32-S3R8, 8 MB PSRAM, 16 MB flash, IMU, RTC, microphone/audio codec, battery management, speaker connector, TF slot and OV5640-compatible camera interface on one 92.44 x 61.00 mm board. This eliminates the original separate TFT + ESP32 board + IMU + mic + audio amplifier wiring for the Lite build.

## 3. Nominal envelope
- Body: 126 mm tall, 158 mm bottom width -> 146 mm top width, 108 mm bottom depth -> 98 mm top depth.
- Head: 90 mm tall, 128 mm top width -> 120 mm bottom width, 40 mm deep.
- Stationary base plate: 170 x 120 x 2.0 mm.
- Fixed head angle: nominal 7 degrees upward.
- Estimated bare aluminum mass: about 0.7-0.8 kg before electronics, fasteners and battery.

## 4. Material
Preferred shell: 5052-H32 aluminum, 1.5 mm nominal. 5052 is preferred for enclosure work because it bends without cracking as readily as 6061 and is readily welded/machined. If 1.5 mm is not locally available, 1.3 mm 5052-H32 is acceptable for this stationary Lite shell. Base plate: 2.0 mm aluminum.

## 5. Head / screen
Board envelope: 92.44 x 61.0 x 11.5 mm. Active area: 73.44 x 48.96 mm. Front opening is intentionally 75.5 x 51.0 mm, slightly larger than the active pixels but still inside the black glass border.

Board mounting pattern from the published Waveshare mechanical drawing: four M2 mounting points, approximately 72.00 mm horizontal spacing and 48.50 mm vertical spacing, centered in the 92.44 x 61.00 mm board. Use a removable inner carrier and slotted holes; do not weld the board directly into the head.

## 6. Camera
A 16 x 12 mm top-center camera window is reserved above the display. Candidate A is the OV5640 module bundled/supported by Waveshare. Autofocus is optional only if the exact module pinout is confirmed compatible. The camera window is intentionally larger than the lens so the shell is not tied to one lens housing.

## 7. RF - critical aluminum-body fix
An all-aluminum head can severely attenuate the onboard 2.4 GHz antenna. The design therefore reserves a non-metallic RF window on the head rear/right area. First test the onboard antenna with this window. If range is poor, the board has an IPEX connector and can be switched to an external 2.4 GHz antenna according to the manufacturer procedure. Do not close the antenna completely behind metal.

## 8. Audio
Use the onboard microphone first and the included 6 ohm / 1 W speaker. The head includes an acoustic vent zone; the body bottom includes a hidden speaker outlet. If microphone pickup is weak, add a short acoustic channel or an external MEMS mic later without changing the shell.

## 9. Emotion RGB
Use one cheap diffused common-cathode RGB LED. Suggested first wiring: GPIO39 -> 330 ohm -> Red, GPIO40 -> 330 ohm -> Green, GPIO41 -> 330 ohm -> Blue, common cathode -> GND. PWM controls brightness/color. The front body uses seven 1.2 mm micro-holes plus an internal diffuser/baffle so the light is subtle when off.

## 10. Power
Phase 1: power only by USB-C while software is being developed.
Phase 2 battery candidate: protected 1S 3.7 V LiPo, 4000 mAh nominal, maximum preferred envelope 95 x 55 x 12 mm, connected only to the board's MX1.25 2-pin battery header after connector polarity is verified. The body reserves 100 x 60 x 15 mm. A 5000 mAh pack is acceptable only if it physically fits and uses the correct protected 1S chemistry/connector.

The future full mobile robot still uses the separate higher-voltage traction battery architecture. This 1S Lite battery is not the future wheel/arm traction pack.

## 11. Upgrade interfaces preserved
- Bottom: 4 x M4 future base pattern, 120 x 70 mm. The stationary foot plate can later be removed and replaced by a moving chassis.
- Top: 4 x M4 head-module pattern, 50 x 30 mm + 22 mm central cable pass. The fixed neck can later be removed and replaced by Pan/Tilt/Lift.
- Belly: internal free zone is preserved so the single RGB can later become the hidden matrix.
- Upper side body: shoulder keep-out zones remain uncut for future arms.

## 12. Freeze / do-not-freeze
Frozen for the shell: overall Lite envelope, removable rear panels, board clearance, RF window requirement, bottom/top upgrade interfaces.
Not frozen until parts are in hand: exact camera bracket, mic acoustic duct, speaker clip/holes, battery strap spacing, USB panel extension, final cosmetic finish.


---

# Bill of Materials

```csv
Category,Part,Qty,Stage,Notes
Main integrated board,Waveshare ESP32-S3-Touch-LCD-3.5B,1,BUY AFTER BODY/SOFTWARE PLAN,3.5in touch + ESP32-S3 + audio + IMU + RTC + PMIC
Camera,OV5640 DVP module compatible with board,1,BUY WITH/AFTER MAIN BOARD,Bundle camera preferred for lowest integration risk
Speaker,6 ohm 1 W speaker (included),1,INCLUDED,Use board speaker header
Emotion LED,5mm diffused common-cathode RGB LED,1,CHEAP,3x 330R series resistors
RGB resistors,330 ohm 1/4W,3,CHEAP,Tune later if needed
Battery,"Protected 1S 3.7V LiPo 4000mAh, <=95x55x12mm",1,LATER,Optional; USB-C first
Shell,5052-H32 aluminum sheet 1.5mm,~0.25 m2,FABRICATE,1.3mm acceptable fallback
Stationary base,Aluminum sheet 2.0mm 170x120mm,1,FABRICATE,4 rubber feet
Rear service hardware,M3 screws + rivnuts/nuts + angle tabs,set,FABRICATE,Body and head rear panels removable
Board mounts,M2 standoffs + insulating washers,4,AFTER BOARD,72 x 48.5mm board hole spacing
RF window,Polycarbonate/ABS/acrylic 35x20mm,1,FABRICATE,Required in aluminum head
Cable protection,Rubber grommet / edge trim / heatshrink,set,FABRICATE,No wire on raw aluminum edges
Feet,Rubber feet,4,CHEAP,Stationary first build
```

---

# Design Parameters

```json
{
  "version": "0.1",
  "shell_material": "Aluminum 5052-H32 preferred",
  "shell_thickness_mm": 1.5,
  "base_plate_thickness_mm": 2.0,
  "body_height_mm": 126,
  "body_bottom_width_mm": 158,
  "body_top_width_mm": 146,
  "body_bottom_depth_mm": 108,
  "body_top_depth_mm": 98,
  "head_height_mm": 90,
  "head_top_width_mm": 128,
  "head_bottom_width_mm": 120,
  "head_depth_mm": 40,
  "head_nominal_tilt_deg": 7,
  "base_plate_width_mm": 170,
  "base_plate_depth_mm": 120,
  "display_board_width_mm": 92.44,
  "display_board_height_mm": 61.0,
  "display_board_thickness_mm": 11.5,
  "display_active_width_mm": 73.44,
  "display_active_height_mm": 48.96,
  "display_opening_width_mm": 75.5,
  "display_opening_height_mm": 51.0,
  "camera_window_width_mm": 16,
  "camera_window_height_mm": 12,
  "battery_bay_width_mm": 100,
  "battery_bay_depth_mm": 60,
  "battery_bay_height_mm": 15,
  "future_base_mount_x_mm": 120,
  "future_base_mount_y_mm": 70,
  "head_interface_x_mm": 50,
  "head_interface_y_mm": 30,
  "head_cable_hole_mm": 22
}
```

---

# Aluminum Build Guide

## Material and fabrication strategy
- Preferred: 5052-H32 aluminum, 1.5 mm shell; 2.0 mm stationary base.
- Weld only the structural/cosmetic shell seams. Keep the body rear panel and head rear panel removable with screws.
- Add small internal aluminum-angle tabs/rails for the removable covers.
- Never weld with electronics, camera, speaker or battery installed. Remove all metal chips before installing electronics.

## One-shot fabrication loop
1. Print the 1:1 PDF at **Actual Size / 100%**. Measure the 50 mm calibration line. If it is not exactly 50 mm, stop.
2. Tape the paper to the aluminum and center-punch every marked pilot point.
3. Cut the shell pieces slightly proud if cutting by hand; final-scribe and trim during dry fit. Handmade TIG-welded sheet metal will not hold sub-0.5 mm enclosure tolerances without a jig.
4. Dry-clamp the whole body. Check top/bottom diagonals and symmetry.
5. Tack weld opposite corners in alternating order. Do not run a long continuous bead on one side first; heat distortion will move the shell.
6. Recheck the display opening and removable rear-cover fit after tacking.
7. Finish with short alternating welds/stitches. Let the part cool between sections.
8. Grind cosmetic outside seams only as much as necessary. Keep internal corner material where it adds stiffness.
9. Fit rubber feet and the removable stationary base plate.
10. Install insulating board spacers, foam gasket around the display, RF window, microphone vent foam and speaker only after all metalwork is done.

## Electronics insulation
- Keep at least ~2-3 mm clearance between PCB solder joints and aluminum.
- Use M2 standoffs plus insulating washers or a thin FR4/Kapton barrier where needed.
- No loose wire should be able to rub on a sharp aluminum edge; use grommets and edge trim.

## RF window
The ESP32 antenna must not be fully surrounded by aluminum. Fit a 35 x 20 mm polycarbonate/ABS/acrylic window in the marked rear/right head zone. Keep metal and battery wiring away from that zone. If Wi-Fi/BLE remains weak, use the board IPEX external antenna option.

## Serviceability
The rear body and rear head covers are intentionally not welded. The software prototype will be opened many times. This is a feature, not a cosmetic compromise.


---

# ZI-E Lite v0.1 Wiring Guide

## Main board
Waveshare ESP32-S3-Touch-LCD-3.5B. The display, capacitive touch, ESP32-S3R8, 8 MB PSRAM, 16 MB flash, IMU, RTC, microphone, audio codec and power management are already on the board.

## Connections
### 1. Camera
OV5640 module -> board camera FPC connector. Follow the connector orientation shown by Waveshare; do not reverse the flex cable. No breadboard jumper wires are used for the camera bus.

### 2. Speaker
Included 6 ohm / 1 W speaker -> MX1.25 speaker connector on the board. Speaker polarity is not critical for a single speaker. Route the two wires through the fixed neck into the lower body speaker chamber.

### 3. RGB emotion LED
Use a common-cathode diffused RGB LED.
- GPIO39 -> 330 ohm -> R anode
- GPIO40 -> 330 ohm -> G anode
- GPIO41 -> 330 ohm -> B anode
- LED common cathode -> GND
Use PWM; begin with low duty cycle. If a particular LED is too dim, resistor values can be tuned after the optical test. Do not exceed the LED/GPIO current ratings.

Header reference from the board pinout: GPIO39, GPIO40, GPIO41 and multiple GND pins are exposed on the 2.54 mm header.

### 4. Power during software development
Laptop/USB supply -> board USB-C. This is the default for v0.1. Do not add the battery until the software and current draw are known.

### 5. Optional battery later
Protected 1S 3.7 V LiPo -> board MX1.25 battery header. Verify connector polarity with the Waveshare schematic/board marking before plugging it in. Never connect a 2S/3S battery to this connector. Never connect the LiPo to the board 5V pin.

### 6. Optional external antenna
If the closed aluminum head reduces Wi-Fi/BLE range, enable the board's IPEX antenna path using the manufacturer procedure and attach a 2.4 GHz flex antenna behind the non-metallic RF window.

## Cable bundle through the neck
Only these cables need to leave the head in the Lite build:
- 2 speaker wires
- 4 RGB wires (R/G/B/GND) if LED is in body
- 2 battery wires if battery is in body
- optional USB-C extension/cable

Keep the camera FPC entirely inside the head.


---

# Software Bring-up Order

1. Flash factory/demo firmware and verify display + touch + IMU + RTC + audio.
2. Create ZI-E face renderer (LVGL) in landscape orientation.
3. Add touch events: wake, mute, simple settings, debug overlay.
4. Bring up OV5640 capture and local preview.
5. Bring up microphone capture and speaker playback.
6. Add Wi-Fi provisioning and laptop connection.
7. Implement host ZI-E daemon: WebSocket/HTTP status + camera/audio/session link.
8. Connect speech recognition, TTS, LLM and memory on the laptop.
9. Add the single RGB emotion state output.
10. Add diagnostics: board temperature/power state, Wi-Fi RSSI, camera status, audio status, RTC and battery state.
11. Only after the stationary software is stable, start the mechanical upgrade branch.

The API must be hardware-agnostic so the later full robot can keep the same high-level commands while replacing the stationary backend.


---

# Open-source / Pre-purchase Engineering Loop

This Lite branch keeps the project rule used for Reachy, LeRobot, OpenArm, Stretch, MoveIt and Nav2 studies: before buying or freezing hardware, inspect mature open-source implementations, documentation, issue trackers and failure reports for the same class of problem.

Applied fixes in this revision:
- **Integrated electronics instead of many loose modules:** reduces wiring/pin conflicts and gives a documented, repeatable starting platform.
- **Removable rear covers:** software prototypes need repeated access; sealed/welded service panels are avoided.
- **RF window in aluminum head:** a metal enclosure can compromise Wi-Fi/BLE; RF was treated as a mechanical requirement, not an afterthought.
- **No exact camera/mic bracket freeze before parts:** optics and acoustic alignment stay adjustable.
- **Modular base/head interfaces:** this stationary shell can become a software/electronics upper-body test platform after the mobile base and motorized head are developed.
- **USB-first:** avoids buying a battery before measured current/runtime requirements exist.
- **No motion safety complexity in Lite:** because the robot is stationary, cliff sensors, motor drivers and STM32 motion safety are deferred rather than simulated badly.


---

# Decision Log

- 2026-08-23: Start software-first stationary Lite branch.
- 2026-08-23: No 3D printing; body/head fabricated from aluminum sheet.
- 2026-08-23: No arms/wheels/head motors/laser in Lite.
- 2026-08-23: Replace large belly matrix with one hidden RGB LED for Lite.
- 2026-08-23: Use integrated ESP32-S3-Touch-LCD-3.5B as Candidate A to reduce wiring, adapters and pin conflicts.
- 2026-08-23: USB-C power first; battery purchased later after current/runtime measurement.
- 2026-08-23: Preserve removable head/base interfaces for future mechanical upgrade.


---

# Research Sources

1. Waveshare ESP32-S3-Touch-LCD-3.5B product/documentation
   - https://www.waveshare.com/esp32-s3-touch-lcd-3.5b.htm
   - https://docs.waveshare.com/ESP32-S3-Touch-LCD-3.5B
   Key design inputs: 3.5-inch 320x480 capacitive IPS, ESP32-S3R8, 8 MB PSRAM, 16 MB Flash, QMI8658 IMU, PCF85063 RTC, ES8311 audio codec, onboard microphone, AXP2101 power/battery management, camera interface supporting OV5640/OV2640, battery/speaker headers, 92.44 x 61.00 mm bare-board mechanical envelope.

2. Protocase Electronic Enclosure Design 101
   - https://www.protocase.com/blog/2017/02/16/electronic-enclosure-design-101/
   Design input: 5052 aluminum is well suited to sheet-metal enclosures and is easily welded/machined; it bends more reliably than 6061 for enclosure work.

3. Protocase aluminum thickness/tolerance and bend-radius references
   - https://www.protocase.com/resources/tolerances/aluminum.php/1000
   - https://www.protocase.com/resources/bend-radii/
   Design input: real sheet thickness and fabrication tolerances must be accounted for; do not assume perfect nominal stock or zero weld distortion.

4. Adafruit OV5640 camera guide
   - https://learn.adafruit.com/adafruit-ov5640-camera-breakout
   Design input: parallel camera signals are sensitive to sloppy breadboard wiring; keep the FPC/interface short and use the intended connector.


---

# Repository Map

- `README.md`
- `START_HERE.txt`
- `archive/FULL_V0_3_DEFERRED.md`
- `bom/BOM.csv`
- `cad/README.md`
- `cad/ZI-E_Lite_v0.1_ASSEMBLY.step`
- `cad/body_shell.stl`
- `cad/head_shell.stl`
- `cad/stationary_base.stl`
- `design_params.json`
- `docs/ALUMINUM_BUILD_GUIDE.md`
- `docs/DECISION_LOG.md`
- `docs/MASTER_SPEC.md`
- `docs/OPEN_SOURCE_HARVEST_LOOP.md`
- `docs/RESEARCH_SOURCES.md`
- `docs/ZI-E_Lite_v0.1_BUILD_AND_WIRING_GUIDE.pdf`
- `electronics/WIRING_DIAGRAM.svg`
- `electronics/WIRING_GUIDE.md`
- `renders/ZI-E_Lite_v0.1_preview.png`
- `software/README.md`
- `templates/CUT_LIST.csv`
- `templates/ZI-E_Lite_v0.1_1to1_A4_CUT_TEMPLATES.pdf`
- `templates/dxf/BATTERY_CARRIER.dxf`
- `templates/dxf/BOARD_CARRIER.dxf`
- `templates/dxf/BODY_BOTTOM.dxf`
- `templates/dxf/BODY_FRONT.dxf`
- `templates/dxf/BODY_LEFT_SIDE.dxf`
- `templates/dxf/BODY_REAR.dxf`
- `templates/dxf/BODY_RIGHT_SIDE.dxf`
- `templates/dxf/BODY_TOP.dxf`
- `templates/dxf/HEAD_BOTTOM.dxf`
- `templates/dxf/HEAD_FRONT.dxf`
- `templates/dxf/HEAD_LEFT_SIDE.dxf`
- `templates/dxf/HEAD_REAR.dxf`
- `templates/dxf/HEAD_RIGHT_SIDE.dxf`
- `templates/dxf/HEAD_TOP.dxf`
- `templates/dxf/STATIONARY_BASE.dxf`
- `templates/svg/BATTERY_CARRIER.svg`
- `templates/svg/BOARD_CARRIER.svg`
- `templates/svg/BODY_BOTTOM.svg`
- `templates/svg/BODY_FRONT.svg`
- `templates/svg/BODY_LEFT_SIDE.svg`
- `templates/svg/BODY_REAR.svg`
- `templates/svg/BODY_RIGHT_SIDE.svg`
- `templates/svg/BODY_TOP.svg`
- `templates/svg/HEAD_BOTTOM.svg`
- `templates/svg/HEAD_FRONT.svg`
- `templates/svg/HEAD_LEFT_SIDE.svg`
- `templates/svg/HEAD_REAR.svg`
- `templates/svg/HEAD_RIGHT_SIDE.svg`
- `templates/svg/HEAD_TOP.svg`
- `templates/svg/STATIONARY_BASE.svg`