# ZI-E Master Specification — 2026-08-23

## 1. Source-of-truth rule
ZI-E is an original desktop AI companion robot. This file describes the current approved architecture. Legacy WALL-E files are historical reference only and may contain contradictions, pin conflicts, or obsolete assumptions. Current CAD, current-state docs, verified component datasheets, tests, and approved decision logs override legacy material.

## 2. Design intent
- Desktop-sized, compact, premium/cute/futuristic robot; not a WALL-E/BMO copy.
- One landscape digital face screen; hidden camera brow; clean exterior.
- Structural chassis carries loads; cosmetic shell is removable/protective.
- Buildable, serviceable, modular, testable, and upgradeable.
- Exact final external dimensions remain subordinate to real component packaging, swept volumes, mass/COM, cable routing, and testing.

## 3. High-level intelligence architecture
`Laptop / future phone AI brain ↔ Wi-Fi ↔ ESP32-S3 multimedia controller ↔ classical CAN ↔ STM32 safety/motion controller`.

The AI layer sends high-level intents only. It never receives unrestricted raw actuator authority. The local safety/motion controller enforces limits, interlocks, heartbeats/timeouts, sensor plausibility, current budgets, and safe-stop behavior.

## 4. Main electronics
### Multimedia controller
- ESP32-S3-WROOM-1-N16R8.
- Responsibilities: display/UI/face animation, OV5640 camera, touch, audio path, Wi-Fi/BLE, laptop/phone communications, high-level robot state.
- 16 MB flash / 8 MB PSRAM class.

### Safety / motion controller
- STM32G0B1RET6.
- Responsibilities: deterministic motion, safety supervision, sensor fusion, actuators, limits, current/fault supervision, cliff/collision handling, heartbeat/command lease, safe-state transitions.

### Main inter-controller link
- Classical CAN/TWAI. CAN-FD is not required for the ESP32-S3 link.

### IMU
- LSM6DSOX primary candidate; BMI270 backup.
- Mounted to rigid chassis near robot center, isolated from avoidable vibration/magnetic interference.

## 5. Face / camera / audio
### Display
Primary candidate: 3.5-inch IPS TFT, 320×480 native (used landscape as 480×320), ILI9488, capacitive touch, SPI/parallel capable.
- Final mode starts with SPI to conserve GPIO and simplify integration.
- Before purchasing/final PCB freeze: exact FPC pinout and exact capacitive-touch controller must be confirmed.
- Backup A: documented EastRising/BuyDisplay 3.5-inch IPS touch panel in similar envelope.
- Backup B: equivalent 3.5-inch IPS touch panel with documented pinout and replaceable adapter frame.

### Camera
- OV5640 5 MP autofocus primary; OV2640 budget fallback.
- Camera sits top-center in structural sensor brow.
- Aesthetic smoked cover is allowed only if low-light/vision testing remains acceptable.

### Dedicated laser rangefinder
- Primary: VL53L1X single-point Time-of-Flight sensor, invisible 940 nm Class 1 laser, up to 4 m / 50 Hz.
- Prototype CAD reserves a 25.5 × 17.5 × 4.6 mm carrier; final head sensor PCB may use the 4.9 × 2.5 × 1.56 mm bare package.
- Located in the modular head sensor brow beside the centered OV5640; it does not replace the lower STM32 cliff/proximity safety sensors.
- Primary role: requested distance measurement, camera/object distance assistance, approach measurement and vision calibration.
- The sensor is single-point: camera/rangefinder extrinsic alignment must be calibrated and software must expose the actual ranging point.
- Optical cover must be qualified around 940 nm, with minimized air gap/tilt and offset/crosstalk calibration after assembly. Arbitrary smoked plastic is not accepted without measurement.
- Backup A: Benewake TF-Luna 0.2–8 m / up to 250 Hz if field requirements exceed 4 m or 50 Hz.
- Backup B: TFmini-S class only if a real longer-range requirement appears.
- Detailed rules: `docs/LASER_RANGEFINDER_SPEC.md`.

### Microphones
- 2 matched I2S MEMS microphones in the head brow, symmetric around camera.
- Current candidate: PUI DMM-4026-B-I2S-R; TDK T5848 class is a backup.
- Do not expand to four microphones unless A/B acoustic testing proves a real benefit.

### Speaker
- One full-range speaker in isolated lower-front/slightly-side acoustic chamber.
- Current candidate class: PUI AS04004PO-R (~40 mm, 4 Ω / 2 W class); smaller AS02404PO fallback if CAD space requires.
- MAX98357A I2S class-D amplifier.
- Speaker stays out of head to minimize head mass and microphone/camera vibration.

## 6. Head mechanics
The head has three controlled DOF/modes: Pan, Tilt, and vertical retract.

### Pan
- SC09 smart servo primary, short 1:1 timing belt, external bearing-supported pan ring with hollow cable center.
- Limited rotation, roughly ±120° design envelope; no continuous 360°/slip ring.
- ST3036 class backup if jitter/backlash is unacceptable.

### Tilt
- SC09 primary with about 2:1 belt reduction; dual external bearings carry head load.
- Pivot placed close to measured head COM.
- ST3036 backup for precision; HL2915 backup for higher torque.
- Small counterbalance spring may be added after measured head mass/COM.

### Lift
- N20 12 V ~200 RPM encoder gearmotor + T5×2 lead screw.
- One precision rail with two spaced carriage blocks plus secondary anti-rotation guide.
- Top and bottom sensors, encoder, mechanical hard stops, current-based jam detection, and manual screw recovery.
- Counterbalance spring target is set only after actual head weighing; do not assume lead-screw self-locking.

### Head operating states
- Active: head raised, Pan/Tilt enabled, camera/display operational.
- Sleep: head lowered enough to hide neck while most face remains visible.
- Protected Shutdown: Pan centers, Tilt moves to dock angle, positions are confirmed, then head retracts deeply/flush; loads rest on three structural supports with compliant pads; no structural latch.

## 7. Body / chassis / service architecture
- Central Head Bay is a strict keep-out volume.
- Left/right structural shoulder towers form a portal around Head Bay.
- Rear/lower bridge and lower chassis carry loads.
- Battery is low and central.
- Rear service shell is sculpted, removable, cosmetic/protective, with no essential wiring attached to it.
- Precision structures remain aligned when the shell is removed.
- Immediate rear access targets: master disconnect/fuse, power board, controller/connectors, debug/service interfaces.
- Internal rails/adapter plates should allow future boards/actuators without re-cutting the entire chassis.

## 8. Arms
Current chain per arm:
`2-DOF bearing-supported Shoulder → proximal belt-driven Elbow → one-stage keyed Telescopic Forearm → limited bearing-supported Forearm Roll → dual-bearing Wrist Pitch → Quick-Swap → Tool`.

### Shoulder
- 2× STS3215-C018 12 V smart servos per arm, motors close to body.
- Gravity-loaded axis may use ~1.3–1.5:1 reduction; second axis may stay near 1:1.
- External bearings/axles carry loads; servo does not act as structural bearing.
- Final status remains conditional on measured/dummy-arm load and thermal tests.

### Elbow
- HL2915-C001 12 V primary, mounted proximally in upper arm, short toothed belt about 2:1 to a dual-bearing elbow axle.
- STS3215 fallback if physical torque/thermal margin is inadequate.

### Telescopic forearm
- N20 12 V encoder gearmotor + T5×2 lead screw + POM nut initially.
- Keyed/non-round sliding geometry and two spaced guide/wear regions carry lateral loads.
- Add thrust bearing so lead-screw axial load does not enter gearbox.
- Retract and extend sensors plus physical stops; current+encoder jam logic.
- Stroke is not frozen until full arm packaging/stability analysis.

### Forearm roll
- SC09 primary, short reduction and hollow bearing-supported output shaft.
- Rotation is limited, not continuous, to protect cables.
- Use optical/mechanical index if needed; avoid Hall reference near magnetic tool.
- ST3036 class backup if load/precision tests fail.

### Wrist pitch
- SC09 primary with about 2:1 reduction and dual external bearings.
- Designed around ~150 g tool class initially; heavier tools trigger stronger backup such as ST3036/HL2915 and/or lower allowed payload.

### Arm safety/stow
- Sequential stow: tool safe → wrist → roll → telescope retract → elbow → shoulders/body dock.
- Each stage confirms previous state; no force-through on failed confirmation.
- Structural docking pockets and compliant pads carry stowed arm loads.
- Collision model includes head safety volume, body, left/right links, and tool envelopes.
- Payload/speed/base-motion limits change with extension and tool metadata.

## 9. Default gripper tool
- 2-finger adaptive underactuated gripper, one SC09 actuator.
- Tendon/differential mechanism, two rigid phalanges per finger, return torsion springs.
- Spring-open/fail-release behavior on power or tendon failure.
- Replaceable soft TPU/silicone pads, rounded precision/V-groove tips.
- Series-elastic tendon element limits pinch force; servo feedback is not treated as a calibrated force sensor.
- Future tactile sensors are reserved but not mandatory for V1.

## 10. Magnetic pickup tool
- Dedicated quick-swap tool rather than a strong permanent palm magnet.
- P20/15 12 V electromagnet class is current prototype candidate.
- Local MOSFET drive, temperature sensing, contact/part detection, compliant/floating mount, protective ring.
- Marketing holding-force numbers are not treated as payload ratings; real pull-force versus material/thickness/air-gap/temperature must be bench measured.
- Default is fail-release electromagnet for V1; electro-permanent magnet remains a future alternative if requirements change.

## 11. Quick-swap interface
- 3-point tapered/self-centering seating.
- Asymmetric key.
- 3-lug short-twist bayonet primary lock.
- Spring safety collar; manual release requires pull + twist.
- Structural interface carries load; contacts carry no structural load.
- 8 contacts: 2× VTOOL+, 2× GND, RS485_A, RS485_B, passive TOOL_ID, AUX.
- Independent left/right wrist RS-485 links.
- Tool power remains off until lock confirmed and tool ID is plausible.
- Opening collar cuts tool power before rotation/unmating.
- Each tool has a small local MCU (STM32C011-class) and independent transceiver/driver electronics as required.


## 11B. Hidden Belly Light Matrix (BLM)
- Fixed-torso low-resolution RGB information/ambient-light surface, hidden when off.
- Prototype electronics: IS31FL3741 13×9 / 117 RGB pixel class; final custom PCB may be wider within the reserved CAD keepout.
- Current CAD v0.3 reserves an 80 × 54 × 12 mm electronics/optical keepout and a flush ~74 × 46 mm optical insert.
- Optical insert is secret-until-lit PC/PMMA with body-matched finish/coating; an opaque normal shell cannot provide the effect.
- Internal black baffle/light well suppresses crosstalk and seam glow.
- Controlled by ESP32-S3 on a non-safety bus/power path. The matrix may fail dark without affecting safety.
- Hardware enable/load switch and global current/brightness cap are mandatory.
- Intended information includes clock, timer, battery, charging, tool state, warnings, directional cues, AI state, audio visualization and ambient effects.
- Safety/warning presentation has priority over cosmetic AI animation.
- Final optical material, transmission, diffuser spacing, brightness and thermal limit require bench coupon testing before custom PCB/panel purchase.

## 12. Mobility
- Exactly 2 independently powered fixed wheels, differential drive. Not four powered wheels, not mecanum/omni drive.
- Hidden passive ball caster provides the third normal support point.
- Front/rear passive anti-tip skids catch excessive tipping.
- Wheels manually fold/tuck for Protected Shutdown; robot then rests on four fixed rubber feet.
- Wheel module: 25 mm-class encoded gearmotor → short timing belt → independent axle on two bearings → replaceable soft tread wheel.
- Current motor candidate: 25SG-370CA-78-EN 12 V ~108 RPM no-load class, with encoder.
- Current driver: one DRV8876 per wheel with current sensing/regulation.
- Drive is disabled unless both wheel deployed locks are independently confirmed.
- Final wheel diameter/tread/ratio remain physical test items; ~60–65 mm is only the present class/envelope.

## 13. Desk-edge and collision safety
### Cliff sensing
- Current architecture: 4× downward VL53L4CD, one near each lower corner.
- These are local STM32 safety inputs, not AI-only perception.
- Invalid/stale critical cliff reading causes stop.
- Mandatory physical tests include white, matte black, glossy black, wood, metal, mirror/reflective surfaces, clear glass, tablecloth/edge overhang, sunlight/darkness, dirty/blocked/unplugged sensors, rounded edges, and multiple payload configurations.
- If optical common-cause blind spots remain, add independent mechanical cliff feelers rather than simply adding more of the same optical technology.

### Horizontal collision
- Current design reserves four lower-body horizontal VL53L4CD-class proximity sensors; this is conditional on cost/space.
- Cost fallback: two horizontal ToF plus bumper ring/camera, but downward cliff sensing is not reduced for cost.

### Physical bumper
- Thin compliant lower bumper with four independent microswitches, current candidate Omron D2F-L class; sealed D2HW-class backup if needed.
- Bumper hit is handled locally by STM32.

## 14. Battery, protection, charging, and rails
### Battery
- 3S2P 18650 Li-ion pack, Molicel M35A candidate.
- Nominal pack class ~10.8 V, full 12.6 V, ~7 Ah / ~75 Wh class.
- Low-central longitudinal battery cassette with structural rails, hard stop, positive rear latch, keyed connection, elastomer anti-rattle pads, and service/removal access.
- No direct hand-soldering to cells; final pack should be professionally/appropriately assembled with proper interconnects and protection.

### Protection
- BQ7791500-class 3–5S protection/balancing family is current direction; exact sense resistor, MOSFETs, fuse, and thermistor implementation freeze only after measured current profile and pack tests.
- Main fuse/service disconnect remain mandatory.

### Charging V1
- External certified 12.6 V CC/CV charger, around 2 A class.
- Charging connected disables drive/arms.
- Integrated USB-C PD charging remains a later polished-revision option.

### Power rails
- Raw 3S rail for suitable 12 V-class motors/actuators, under branch protection/current budgets.
- AP62300-class buck converters for general 5 V / 3.3 V rails where suitable.
- Independent TPS62162-class 3.3 V safety rail for STM32/critical sensors.
- 6 V arm/head servo rails sized to real grouped stall/thermal behavior; current arm design uses a higher-current ~6 A-class buck per arm plus upstream current limiting/eFuse.
- TPS25947-class eFuse/current-limited branches for arms/tools where applicable.
- Motion Kill cuts actuator enables/rail authority without killing safety diagnostics.

## 15. Physical controls and safety states
- Physical Power control.
- Hardware Motion Kill.
- Hardware Privacy control intended to power-disable camera/microphones where practical.
- States: OFF → BOOT/SELF-TEST → SAFE_IDLE → ACTIVE → MANIPULATION → SLEEP → PROTECTED_SHUTDOWN, plus CHARGING/SERVICE/FAULT.
- Transitions are gated by actual confirmed state, not command assumptions.
- Wi-Fi/laptop loss, stale command lease, impossible sensor state, overcurrent, jam, critical cliff fault, or safety-controller fault triggers locally defined degraded/safe behavior.

## 16. Software / AI behavior boundary
- AI: vision, speech, language model, memory/database, behavior selection, high-level task planning.
- Local controller: deterministic motion and safety.
- Early “learning” means stored memory/retrieval/adaptation, not autonomous continuous model retraining on the robot.
- Local ring-buffer/flight-recorder fault logging is required.
- Updates and communications must eventually use authenticated/encrypted channels with rollback-safe update architecture.



### Commissioning / execution semantics added from open-source failure review
- First power-up uses a low-current commissioning state: scan actuator IDs/model/baud, verify voltage class, direction, physical indexing, zero/range and feedback one joint at a time. No autonomous animation before this passes.
- A motion request has explicit states such as RECEIVED / ACCEPTED / ARMED / EXECUTING / COMPLETED / REJECTED / FAULTED. Receipt is never treated as proof of motion.
- One command arbiter owns each moving subsystem; AI, web UI, teleoperation and scripted behavior cannot fight over raw setpoints.
- Servo/head/arm cable topology avoids unnecessary downstream dependence on pass-through connectors; left/right/head remain separately isolatable fault domains.
- Dynamic payload/tool mass and COM feed stability and torque limits.
- Dynamic collision zones expand with speed, arm extension, tool length, payload and head height; stale critical range data means stop.

## 17. Thermal / EMI / RF / wiring
- Separate high-current/noisy power routes from sensitive data/audio/RF routes.
- Keep RF antenna area in an upper/rear nonmetallic region away from battery/large metal structures.
- Fixed chassis-mounted motor drivers and power electronics; dynamic wiring only where motion requires it.
- Controlled flex loops for Tilt, Pan center, vertical lift, arm telescope, and wheel folding.
- Strain relief and minimum bend radius are CAD keep-outs, not afterthoughts.
- Passive thermal path must work even in Protected Shutdown; add fans only if measured thermal tests prove necessary.

## 18. CAD status
Current project/CAD release is **v0.3** and is in `mechanical/cad/current/`.
- Parametric source: `src/zie_cad.py`.
- STEP/STL/glTF exports provided.
- Active, Sleep, Shutdown and Cutaway states included.
- Current automated validation snapshot (CAD v0.3): 39 checks total; 38 pass; one case (two arms each carrying 250 g forward) is intentionally restricted for normal driving because static stability margin drops below target.
- Current mass estimate: ~2.44 kg, low/medium confidence until real parts and printed masses are measured.
- Current active envelope is about 248 mm wide × 174 mm depth × 282 mm tall in the modeled state; these are CAD-state outputs, not immutable product dimensions.

## 19. Engineering method / mandatory loops
Every significant change is reviewed through: requirements → research → alternatives → FMEA → common-cause → tolerance/manufacturing → abuse/human error → test plan → integration → change-impact → lifecycle/wear → red-team → approval/redesign.

Additional mandatory checks include memory/inventory loop, **Open-Source Harvest Loop (including issue trackers/troubleshooting)**, interface/dependency loop, worst-combination loop, recovery loop, stale-data/timeouts, boot ambiguity, sensor blind spots/common power, STPA/fault-tree thinking for catastrophic outcomes, graceful degradation, calibration drift, logging/observability, latency, mass/space/energy/cost budgets, privacy/cybersecurity, supply-chain/obsolescence, repair/end-of-life, and an experiment ladder from simulation to controlled physical integration.

## 20. Unresolved / freeze-gate items
Do not call these production-frozen until physically verified:
- Exact selected display FPC/pinout/touch-controller.
- Measured head mass/COM and final Tilt/Lift spring values.
- Dummy-arm/full-arm torque, backlash, thermal, and endurance results.
- Exact telescope stroke and cable flex life.
- Wheel tire diameter/material, traction and scrub behavior.
- Cliff performance on glass/black/mirror/edge cases.
- Electromagnet real pull force and temperature.
- Final BMS shunt/MOSFET/fuse sizing from measured current profiles.
- Acoustic chamber tuning and microphone AEC/beamforming performance.
- Cable bend radii and service life.
- Final external shell dimensions/industrial-design surfaces after physical package validation.

## 21. Prototype experiment ladder
1. CAD geometry/swept-volume/collision checks.
2. Cheap dummy geometry and weighted mechanical rigs.
3. Powered bench rigs for each mechanism.
4. Endurance cycling.
5. Jam/power-loss/misalignment/fault injection.
6. Subsystem integration.
7. Full robot tethered/contained tests.
8. Normal prototype operation after safety gates pass.

Never use uncontrolled home tests for deliberate lithium thermal runaway or similarly hazardous fault creation.
