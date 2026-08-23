# ZI-E v0.3 — MASTER ALL-IN-ONE PROJECT DOCUMENT

This human-readable file consolidates the current source-of-truth documentation. Binary CAD, STL, STEP, glTF, images, legacy files, and archived revisions remain included separately in the same v0.3 release ZIP.

**Release:** v0.3 — 2026-08-23

**New in v0.3:** dedicated head VL53L1X Class-1 ToF laser rangefinder + full integrated release package.



---

# PROJECT LANDING


> Source file: `README.md`


# ZI-E

**Project Release v0.3** — Desktop AI Companion Robot

Includes the dedicated head laser ToF rangefinder and current CAD/pre-purchase design state.

ZI-E is an original, modular desktop AI robot combining an expressive digital face, camera/audio interaction, two functional arms with quick-swap tools, two-wheel differential mobility, local safety control, and an external laptop/future-phone AI brain.

## Current status
**Prototype component architecture + parametric CAD are complete enough to enter physical validation.** Production dimensions/parts are not frozen until the listed freeze-gate tests pass.

### Start here
- [`docs/ZI-E_MASTER_SPEC.md`](docs/ZI-E_MASTER_SPEC.md) — complete current architecture
- [`docs/COMPONENT_MASTER_MATRIX.md`](docs/COMPONENT_MASTER_MATRIX.md) — primary/backups/switch conditions
- [`docs/CURRENT_STATE.md`](docs/CURRENT_STATE.md) — project status
- [`docs/ENGINEERING_METHOD_AND_MEMORY_LOOP.md`](docs/ENGINEERING_METHOD_AND_MEMORY_LOOP.md) — mandatory design/review process
- [`mechanical/cad/current/`](mechanical/cad/current/) — current CAD source, STEP/STL, renders and validation data
- [`docs/OPEN_ITEMS_AND_FREEZE_GATES.md`](docs/OPEN_ITEMS_AND_FREEZE_GATES.md) — what must still be physically verified
- [`docs/GITHUB_HANDOFF_CURRENT.md`](docs/GITHUB_HANDOFF_CURRENT.md) — migration/upload guide

## Key architecture
- AI brain: laptop initially; phone may later become portable/edge brain.
- Multimedia/UI: ESP32-S3-WROOM-1-N16R8.
- Real-time safety/motion: STM32G0B1RET6.
- Face: 3.5-inch landscape IPS touch panel class + OV5640 AF camera.
- Head: Pan + Tilt + vertical retract with Active/Sleep/Protected Shutdown.
- Arms: 2-DOF shoulder, elbow, telescopic forearm, roll, wrist pitch, quick-swap tool.
- Tools: adaptive gripper and removable electromagnet tool.
- Mobility: exactly **two powered wheels + one passive ball caster**; no four-wheel/mecanum base in V1.
- Safety: local cliff sensors, bumper, IMU, current/jam supervision, hardware Motion Kill, state interlocks and safe-stop behavior.
- Battery: low-central 3S2P 18650 Li-ion pack architecture.

## Important warning
`legacy/` contains old WALL-E-era material with known contradictions and is **not source of truth**. Likewise, `assets/deprecated/` contains generated images that may show incorrect geometry. Use current docs/CAD instead.


## Latest design delta
CAD v0.3 includes the hidden **Belly Light Matrix** (secret-until-lit RGB information/ambient panel) and integrates a pre-purchase Open-Source Harvest audit. See `docs/BELLY_MATRIX_SPEC.md`, `docs/OPEN_SOURCE_HARVEST_AUDIT.md`, and `docs/PRE_PURCHASE_GATE.md`.



---

# PROJECT CONTEXT


> Source file: `PROJECT_CONTEXT.md`


# ZI-E Project Context

## Project intent

ZI-E is a **desktop AI companion robot** intended to feel alive, useful, upgradeable, serviceable, and visually distinctive without becoming unnecessarily mechanically complex.

The project will be prototyped comprehensively before the full hardware purchase. GitHub should be the persistent project memory so work can migrate between AI accounts and tools without losing decisions.

## Core philosophy

- Design first, but remain component-aware.
- Avoid committing to hardware before physical and functional requirements are clear.
- Prefer modularity and serviceability.
- Keep the exterior clean while making the interior easy to access and upgrade.
- Minimize unnecessary mechanical complexity.
- Treat AI as a high-level intelligence layer, not as a raw hardware driver.
- Local firmware must enforce safety limits.
- Preserve future upgrade paths without over-engineering Version 1.

## AI / compute direction

The initial intelligent system is not intended to run entirely inside the robot.

### Initial architecture

```text
ZI-E Robot Body
  sensors / camera / microphone / actuators
                 │
              Wi-Fi
                 │
                 ▼
Laptop AI Brain
  vision / speech / LLM / memory / database / behavior
```

The laptop is initially responsible for the heavy AI workload and long-term memory.

### Future architecture

Once the system has a useful database and stable behavior, a **used phone** may become an edge brain / portable compute layer and reduce dependence on the laptop.

The phone may later provide compute, storage, connectivity, camera/IMU/GPS resources, or serve as a bridge to cloud AI.

## Project management direction

The user is the product owner.

ChatGPT acts as the coordinating architect/reviewer: organize work, break it into tasks, prepare prompts for other AI systems, review outputs, avoid duplicated work, manage context, and protect the source of truth.

Other AI systems may include Codex, Claude, and Gemini. They should be assigned narrowly scoped work with clear ownership rather than all editing the same subsystem simultaneously.

## Migration requirement

The project must remain portable to another ChatGPT account later.

Therefore:
- important decisions belong in this repository;
- AI chats are not the sole project memory;
- current state must be maintained in `docs/CURRENT_STATE.md`;
- decisions must be recorded in `docs/DESIGN_DECISIONS.md`;
- legacy material must remain clearly separated from approved design.


Current process also includes an Open-Source Harvest Loop: inspect mature robotics projects and their real issue trackers before freezing important hardware/software decisions.


## v0.3 addition — head laser distance measurement
Dedicated forward VL53L1X Class-1 ToF rangefinder added to the head sensor brow; it is a high-level measurement/vision aid and does not replace STM32 cliff/bumper safety. See `docs/LASER_RANGEFINDER_SPEC.md`.



---

# AI AGENT RULES


> Source file: `AGENTS.md`


# AGENTS.md — ZI-E AI Collaboration Rules

## Source of truth

This repository is the source of truth.

When a chat, legacy PDF, generated code, or AI output conflicts with an approved repository decision, stop and resolve the conflict before implementation.

## Authority

### Product Owner
The user decides:
- desired behavior;
- visual personality;
- acceptable compromises;
- budget and purchase approval;
- final product direction.

### ChatGPT — Coordinating Architect / Reviewer
Responsibilities:
- maintain project structure and current state;
- split work into small verifiable tasks;
- choose which AI should handle each task;
- write the task prompt;
- review outputs for integration conflicts;
- identify assumptions and unresolved risks;
- minimize duplicate AI effort and wasted credits;
- prevent legacy errors from silently entering the new design.

### Coding / specialist agents
Codex, Claude, Gemini, or future agents may implement or independently review scoped tasks.

No agent owns the whole project by default.

## Work rules

1. One subsystem, one owner at a time.
2. Critical work receives an independent review.
3. Do not ask three models to solve the same easy task.
4. Use additional models when the decision is expensive, safety-critical, or likely to create architecture lock-in.
5. Every implementation task must state:
   - inputs;
   - expected output;
   - files allowed to change;
   - constraints;
   - validation/test criteria.
6. Do not modify unrelated files.
7. Do not treat a generated result as approved until reviewed.
8. Hardware-control code must expose safe high-level commands, not arbitrary raw actuator control to the AI layer.
9. Secrets must never be committed.

## Legacy rule

All files under `legacy/` are untrusted reference inputs.

They may contain:
- contradictory pin mappings;
- incorrect power assumptions;
- incomplete placeholders;
- outdated API assumptions;
- unverified mechanical dimensions.

Verify from primary datasheets and the new design before reuse.

## Credit-efficiency rule

Use the cheapest/lowest-effort suitable model for routine transformations and simple implementation.

Reserve stronger reasoning / independent cross-review for:
- power architecture;
- battery safety;
- motion safety;
- mechanical interference;
- protocol architecture;
- major firmware architecture;
- final purchase BOM;
- major design freeze decisions.

## Conversation archive

After every meaningful ZI-E project session:
- archive the user-visible project discussion under `docs/conversations/`;
- update `docs/CURRENT_STATE.md`;
- promote approved decisions to `docs/DESIGN_DECISIONS.md`;
- do not rely on chat-only memory for important project state.

## Open-source harvest rule

Before freezing an important mechanical, electrical, controls, or AI subsystem, inspect mature open-source robotics projects **including their issue trackers and troubleshooting docs**, not only polished README pages.

Required flow: `problem -> comparable projects -> real failure reports -> adapted ZI-E mitigation -> simulation/bench verification -> decision`.

Record material findings in `docs/OPEN_SOURCE_HARVEST_AUDIT.md`. Do not copy code/CAD into ZI-E until license compatibility and provenance are checked. Engineering ideas may inform the design, but imported artifacts need explicit license/source review.

## Safe commissioning rule

No newly assembled robot may enter normal autonomous motion before a low-power commissioning pass verifies actuator identity, model/voltage class, bus/baud, direction, zero/range, end-state sensors, and plausible feedback. A received command is not considered executed until actual state confirms it.



---

# MASTER SPECIFICATION


> Source file: `docs/ZI-E_MASTER_SPEC.md`


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



---

# COMPONENT MASTER MATRIX


> Source file: `docs/COMPONENT_MASTER_MATRIX.md`


# Component Master Matrix

This is the quick lookup table. `Primary` means current first-choice; `Backup` means an intentional substitute, not an equal recommendation. Prices are deliberately omitted because they are volatile; re-check before purchasing.

| Subsystem | Primary | Backup(s) / Switch Condition | Status |
|---|---|---|---|
| Display | 3.5" IPS touch, ILI9488, 480×320 landscape | EastRising/BuyDisplay or equivalent documented panel if FPC/touch IC/availability fails | KEEP, verify exact panel |
| Belly Light Matrix | IS31FL3741 13×9 RGB prototype + secret-until-lit optical insert | Wider custom IS31FL3741-class matrix after optical/power test | KEEP architecture, VERIFY optics/thermal |
| Camera | OV5640 5MP AF | OV2640 if budget forces it | KEEP |
| Head laser rangefinder | VL53L1X 940 nm Class-1 ToF, up to 4 m | TF-Luna 8 m if >4 m / >50 Hz required; TFmini-S for longer-range requirement | KEEP architecture, VERIFY optics/calibration |
| Multimedia MCU | ESP32-S3-WROOM-1-N16R8 | Same class/PSRAM variant only after pin/resource audit | KEEP |
| Safety MCU | STM32G0B1RET6 | Same-family equivalent only after I/O/timer/CAN audit | KEEP |
| Main MCU link | Classical CAN/TWAI | UART only as development fallback | KEEP |
| IMU | LSM6DSOX | BMI270 | KEEP |
| Downward cliff | 4× VL53L4CD | Add mechanical feelers if optical blind spots remain | KEEP architecture |
| Horizontal proximity | 4× VL53L4CD | 2× if cost/space pressure, bumper remains | CONDITIONAL |
| Bumper | 4× D2F-L class | D2HW sealed class | KEEP architecture |
| Wheel motor | 25SG-370CA-78-EN class, 12V encoded | Equivalent 25mm encoded gearmotor meeting envelope/performance | CONDITIONAL on final mass |
| Wheel driver | 2× DRV8876 | Equivalent protected H-bridge after current/thermal audit | KEEP |
| Ball caster | POM ball-caster cartridge | Pololu #2692 / custom | CONDITIONAL geometry |
| Battery | 3S2P Molicel M35A class | Trusted equivalent 18650 only after current/energy audit | KEEP architecture |
| Pack protection | BQ7791500 class | Exact family variant after pack/electrical design | CONDITIONAL implementation |
| Charger V1 | Certified 12.6V ~2A CC/CV external | Integrated USB-C PD later | KEEP V1 |
| General bucks | AP62300 class | Equivalent efficiency/thermal parts | KEEP class |
| Safety 3.3V | TPS62162 class | Equivalent isolated safety supply | KEEP class |
| Arm/head protection | TPS25947/eFuse class | Equivalent current-limited protected switch | KEEP class |
| Shoulder | STS3215-C018, 2/arm | Ratio/actuator upgrade only if load/thermal test fails | CONDITIONAL physical test |
| Elbow | HL2915-C001 + ~2:1 belt | STS3215 if torque/thermal margin fails | CONDITIONAL physical test |
| Telescope | N20 encoder + T5×2 | Faster N20 / changed lead after speed test | KEEP architecture |
| Forearm Roll | SC09 | ST3036 if precision/load fails | CONDITIONAL |
| Wrist Pitch | SC09 + ~2:1 | ST3036 / HL2915 if tool load fails | CONDITIONAL |
| Pan | SC09 + 1:1 | ST3036 if jitter/backlash unacceptable | KEEP initial |
| Tilt | SC09 + ~2:1 | ST3036 precision / HL2915 torque | CONDITIONAL on head mass |
| Head Lift | N20 encoder + T5×2 | Faster N20 / stepper only if justified | KEEP architecture |
| Gripper | SC09 adaptive tendon differential | Stronger actuator if grasp tests require | KEEP concept |
| Magnetic Tool | P20/15 12V electromagnet class | Larger magnet / EPM only if requirements justify | CONDITIONAL physical test |
| Tool MCU | STM32C011 class | Similar low-cost MCU with required UART/ADC/PWM/watchdog | KEEP class |
| Tool data | Independent RS-485 per wrist | — | KEEP |
| Microphones | 2× DMM-4026 I2S class | T5848; 4-mic only after testing | KEEP initial |
| Speaker | AS04004PO-R class | AS02404PO if packaging forces smaller unit | CONDITIONAL CAD |
| Audio amp | MAX98357A | Equivalent I2S class-D | KEEP |



---

# CURRENT STATE


> Source file: `docs/CURRENT_STATE.md`


# Current State — 2026-08-23

## Phase
Prototype architecture and Project/CAD v0.3 package prepared; next phase is risk-retirement bench rigs / safe commissioning prototypes before bulk component purchase, followed by electronics schematics/PCB and firmware integration.

## What is substantially defined
- Overall structural architecture and service philosophy.
- Digital face / camera / dedicated head laser-rangefinder / audio architecture.
- Hidden secret-until-lit Belly Light Matrix (BLM) for RGB ambient light + low-resolution information such as clock/battery/status.
- ESP32-S3 multimedia and STM32 safety/motion split.
- Head Pan/Tilt/Lift mechanisms and operating states.
- Two-wheel differential base, folding wheel pods, passive caster, shutdown feet.
- Dual functional arm architecture with candidate actuators.
- Adaptive gripper and magnetic quick-swap tools.
- Battery placement/configuration, V1 charging approach and protected power-domain philosophy.
- Cliff/collision sensing layers and safety state machine direction.
- Parametric CAD, STEP/STL exports, CAD FMEA/requirements/validation dataset.

## Current CAD snapshot
- Project/CAD v0.3: 39 automated checks; 38 pass; 1 restricted case.
- Dedicated VL53L1X head laser-rangefinder envelope added beside the centered camera.
- Estimated mass ~2.448 kg pending measured masses.
- Active modeled height ~282 mm; Shutdown modeled height ~182 mm.
- Two-arm simultaneous 250 g forward payload is not allowed during normal driving under the current support geometry.

## Main unresolved gates
See `OPEN_ITEMS_AND_FREEZE_GATES.md` and `OPEN_SOURCE_HARVEST_AUDIT.md`: display exact interface, laser optical-window/crosstalk + camera alignment, safe actuator commissioning, bus/cable fault containment, physical cliff testing, real arm/head mass and torque/thermal tests, wheel traction/tire choice, magnet force/temperature, battery protection sizing, audio/cable-life tests, and belly optical/thermal validation.

## Rule for future work
Every new answer/change must use the Memory/Inventory Loop + full engineering loop + Open-Source Harvest Loop, and update repository source-of-truth docs when approved.



---

# ARCHITECTURE


> Source file: `docs/ARCHITECTURE.md`


# System Architecture — Direction v0.1

## Principle

ZI-E's body should remain a safe real-time robot controller, while heavy AI runs outside the body initially.

## Initial system

```text
                       ┌──────────────────────────────┐
                       │          LAPTOP              │
                       │                              │
 Camera/Audio/Sensors ─► Vision / Speech / LLM       │
                       │ Behavior / Memory / Database │
                       │                              │
 Safe intents/commands ◄                              │
                       └──────────────┬───────────────┘
                                      │ Wi-Fi
                                      │
                       ┌──────────────▼───────────────┐
                       │          ZI-E BODY           │
                       │                              │
                       │ Robot controller             │
                       │ Motor/servo control          │
                       │ Local safety                 │
                       │ Telemetry                    │
                       │ Camera/mic transport         │
                       │ Sensor transport             │
                       └──────────────────────────────┘
```

## Local robot responsibilities

Expected:
- Wi-Fi communications;
- actuator control;
- sensor acquisition;
- camera/microphone data transport;
- telemetry;
- emergency stop;
- collision / motion safety;
- state machine;
- safe command validation;
- local fallback behavior where useful.

Not intended as the primary responsibility:
- main LLM;
- long-term personal memory;
- main vector/database system;
- unrestricted AI decisions;
- cloud secrets embedded in distributable firmware.

## Laptop brain responsibilities

Candidate responsibilities:
- speech recognition;
- text-to-speech orchestration;
- vision inference;
- LLM reasoning;
- long-term memory;
- user preferences;
- episodic memory;
- behavior planning;
- database;
- learning/adaptation logic;
- diagnostics and development tooling.

## Memory direction

Prefer adaptation through memory/retrieval before considering continual model retraining.

Possible memory layers:
1. Immediate/context memory.
2. Episodic interaction memory.
3. Long-term people/preferences/places/configuration/behavior memory.

## Future used-phone integration

A used phone may eventually become:
- portable edge brain;
- local database host;
- connectivity bridge;
- camera/IMU/GPS resource;
- speech/vision processing device;
- cloud AI client;
- replacement or partial replacement for the laptop.

Design should preserve room for this architecture without requiring the phone to be physically installed in Version 1 unless later decided.

## Hardware control safety boundary

External AI must request **high-level intents**.

Good examples:
- `MOVE_FORWARD`
- `TURN_LEFT`
- `STOP`
- `WAVE`
- `LOOK_AT_TARGET`
- `SLEEP`

Avoid exposing uncontrolled raw hardware operations directly to AI, such as arbitrary raw PWM/servo values without local safety checks.


## Hidden Belly Light Matrix
A fixed-torso secret-until-lit RGB matrix is controlled by ESP32-S3 as non-safety multimedia. It has independent power gating/current limiting and must not share a critical failure dependency with cliff/safety sensing.

## Commissioning / arbitration
Normal autonomous motion is locked out until actuator identity/direction/range checks pass. High-level control is single-owner/lease based and motion commands distinguish receipt from physical execution.


## Head measurement sensor
`VL53L1X -> I2C -> ESP32-S3` provides single-point forward distance aligned/calibrated with the head camera. It is non-safety-critical. Lower cliff/proximity sensors remain local to STM32.



---

# DESIGN DECISIONS


> Source file: `docs/DESIGN_DECISIONS.md`


# ZI-E Design Decisions — Concept v0.1

This document captures the design decisions made so far. Dimensions are not final unless explicitly stated.

## 1. Overall category and personality

- ZI-E is a **desktop companion robot**.
- It should not be extremely small or large.
- Exact size is secondary to good proportions, internal service space, battery capacity, and a clean desktop presence.
- Visual direction: **cute + futuristic + premium**, but believable and buildable.
- It should have its own identity rather than being a WALL-E or BMO copy.

## 2. Overall body shape

**Approved direction: Hybrid body.**

- Wider toward the bottom.
- Slightly narrower through the middle.
- Soft transitions toward the upper head opening.
- Rounded, non-boxy edges.
- Front chest is **gently convex** to give the robot personality.
- Rear remains sculpted and attractive; it must not look like a flat electronics access panel.

## 3. Head

### Shape
- Custom soft-trapezoid / futuristic head.
- Wider near the top and slightly narrower toward the bottom.
- Rounded edges.
- Balanced head size, not oversized-cute or tiny.
- Target proportion: roughly **55–60% of body width** as a concept guide.
- Head should be relatively low/wide to suit the landscape face and retraction.

### Face
- **One landscape digital screen.**
- Face is digital; no mechanical eyes or eyelids.
- Custom bezel around the display.
- Digital expressions can change without extra mechanics.

### Camera
- Camera belongs in the **top-center region above the screen**.
- It should be highly concealed.
- Preferred appearance: hidden behind a dark/smoked sensor window or very discreet slit.
- Camera remains usable in Active and Sleep modes.
- It becomes protected/hidden in full shutdown.

## 4. Head and neck motion

### Neck
- One central, short, relatively broad neck.
- Internal wiring should route through the neck where practical.

### Head motion
- Pan.
- Tilt.
- Vertical retract.

### Retraction mechanism
**Approved concept:** a vertical slider on internal guide rails.

Avoid a complicated telescopic neck for Version 1.

## 5. Operating poses

### Active Mode
- Head fully raised.
- Neck visible.
- Pan/Tilt available.
- Full visual interaction.

### Sleep Mode
- Head lowers enough for the neck to disappear.
- Display remains visible.
- Camera remains functional.
- Screen may show sleepy eyes, time, battery, or other low-energy information.
- Arms rest in a relaxed state.
- Hidden RGB heart/core uses a slower, dimmer pulse.

### Protected Shutdown
Goal: **maximum protection + minimum physical volume + attractive compact form**.

- Head retracts significantly deeper than Sleep.
- Screen and camera become physically protected.
- Head should be almost completely hidden inside the body.
- Top opening follows the head's shape.
- Body has a slight inward slope and a subtle protective rim around the head opening.
- Arms retract/fold into side recesses.
- Wheels can be manually tucked/folded inward.
- Robot should resemble a compact protected pod rather than a robot merely switched off.
- RGB heart/core turns off.

## 6. Top body opening

- Opening follows the custom head shape rather than being a generic rectangle.
- Upper surface slopes slightly inward toward the opening.
- Very subtle raised/protective rim.
- Aim for a nearly closed/flush-looking protected shutdown profile.

## 7. Chest light / “heart”

- RGB status/emotional light on the front chest.
- Location should be easy to see.
- **It must be hidden when off.**
- No obvious external heart badge when inactive.
- Preferred implementation concept: internal RGB light behind a flush, frosted/seamless diffuser integrated into the convex chest.
- Active: subtle expressive pulse.
- Sleep: slower/dimmer pulse.
- Shutdown: off.
- Future status colors may indicate charge/errors without making the chest look like a dashboard.

## 8. Drive system

### Approved base architecture
- Two **fixed powered wheels**, independently driven.
- Differential drive.
- Hidden passive support point/caster underneath.
- Zero-radius turning using opposite wheel directions.
- No swerve/360-degree steering in Version 1.
- No motorized ball drive.
- No Mecanum/omni base in Version 1.

Reasoning: sideways travel is not worth the extra mechanical complexity, size, wiring, backlash, and failure points for this companion robot.

The independently moving head can maintain gaze while the body reorients.

### Wheel appearance
- Semi-exposed.
- Roughly half visually integrated into the housing and half visible as a concept direction.
- Medium-to-large visual proportion, approximately **35–40% of body height** as a starting proportion only.
- Style: **hidden mechanical + futuristic**.
- Motors/axles/wiring remain concealed.
- Wheel pods should look designed into the body, not attached afterward.

### Protected shutdown wheels
- Manual inward tuck/fold is acceptable and desired.
- No powered wheel-fold mechanism for Version 1.
- Robot may rest on hidden rubber feet when wheels are tucked.

## 9. Arms

The arms are **functional and expressive**, not decorative only.

### Placement
- Shoulder mounts are high on the sides.

### Joints / functions
- Shoulder joint.
- Elbow joint.
- Telescopic forearm, one extension stage.
- Compact wrist/tool interface.
- Compact adaptive two-finger gripper as the normal/default hand.

### Arm reach
- Normal arm length should look proportional in daily use.
- Forearm extends only when extra reach is needed.
- Concept target: extension to roughly 135–150% of normal length, subject to engineering validation.

### Hand/gripper
- Two-finger adaptive gripper.
- Rounded and compact.
- Designed for light tasks such as:
  - holding small/light objects;
  - pressing buttons;
  - passing an object;
  - holding a stylus through an adapter;
  - pushing/pulling light items.

This is not intended as a heavy industrial manipulator.

### Wrist / tool ecosystem
- Manual quick-swap tool system for Version 1.
- Interface should be designed **auto-ready** for a possible future docking/tool-changing system.
- Future tool examples: stylus, sensor, magnetic pickup, suction tool, small light, custom tools.
- Mechanical lock, power/data contacts, and alignment concept will be engineered later.

### Arm shutdown sequence
1. Retract telescopic forearm.
2. Close gripper.
3. Return wrist to safe position.
4. Fold elbow.
5. Tuck shoulder.
6. Entire arm nests into a shaped side recess.

The recess should look like a natural body design line rather than an ugly cavity.

## 10. Internal service architecture

### Exterior vs structure
ZI-E uses:
- cosmetic outer shell;
- structural internal core/frame;
- modular internal subsystems.

The cosmetic shell should not be the sole load-bearing structure.

### Rear access
- Sculpted rear appearance.
- Rear shell should be removable for major service.
- No essential wiring should live on a removable cosmetic cover where it must be unplugged every time.
- Seams should follow design lines.

### Internal modularity
Use multiple modules placed in their optimal functional locations rather than one giant electronics tray.

Candidate zones:
- top: head lift / head motion;
- center: controller and communication;
- bottom-center: battery/power;
- sides: wheel modules and arm mechanisms;
- front peripheral: hidden RGB/core and forward sensors.

Modules should be removable independently where practical.

## 11. Battery concept

- Large battery capacity is a priority.
- Long runtime matters more than shrinking the body by a few centimeters.
- Battery bay should support later capacity upgrades.
- Battery lives low and central for a low center of gravity.
- Preferred access: **camera-style quick-release battery cartridge/door**.
- Best concept location: lower rear, on a subtly angled flush panel.
- User should not need to turn the robot upside down to access it.
- Door should not visually damage the rear design.

Battery chemistry/cell configuration is NOT yet selected.

## 12. Audio

- No ugly front speaker grille.
- Speaker should be hidden inside the body.
- Use an internal acoustic chamber and carefully designed vents/ducts.
- Consider symmetric output paths and desk-surface reflection.
- Do not assume “more holes = better sound”; final acoustic dimensions depend on the real speaker/enclosure.
- Keep speaker physically separated from the head microphone as much as practical.
- Software echo cancellation may be used later.

## 13. Exterior cleanliness

From the front, visible elements should be minimal:
- digital face;
- subtle camera location;
- hidden-when-off RGB heart/core;
- intentional body surfaces.

Speaker openings and sensors should be hidden in the best functional locations using vents, dark windows, seams, or lower/rear openings.

## 14. Serviceability principle

ZI-E should be upgradeable without redesigning the whole robot.

Examples:
- controller replacement;
- battery upgrade;
- wheel module replacement;
- arm/tool upgrade;
- speaker change;
- added sensor;
- future phone integration.

Prefer standardized internal mounting patterns and connectors.

## 15. Concept-image status

The repository contains concept sheets under `assets/concepts/`.

Important correction applied to later concept:
- camera made much less visible;
- RGB chest light made concealed/seamless when off.

Images are **visual direction references**, not dimensionally accurate CAD.


## 2026-08-23 — Hidden Belly Light Matrix and pre-purchase failure-harvest
**Decision:** Add a hidden RGB belly matrix behind a flush secret-until-lit optical insert. Prototype with IS31FL3741 13×9, but reserve larger custom matrix keepout in CAD. It is non-safety multimedia and must fail dark.

**Decision:** Add a permanent Open-Source Harvest Loop. Reachy Mini, LeRobot SO-101, OpenArm, Stretch, MoveIt/Nav2 and MuJoCo failure patterns are used to create preventive tests and architecture changes before bulk part purchase. Immediate changes include safe commissioning, explicit command execution state, single motion arbiter, bus/cable fault containment, dynamic payload/stability model, dynamic collision zones, diagnostic tooling and explicit simulation inertials.

## 2026-08-23 — Project v0.3 dedicated head laser rangefinder
- Add one dedicated forward single-point Class-1 ToF laser rangefinder in the head sensor brow.
- Primary candidate: VL53L1X, because up-to-4 m / 50 Hz is sufficient for a desktop companion and the final bare package is extremely small.
- Camera remains centered; rangefinder is offset and software must calibrate the camera/rangefinder extrinsic relationship.
- Rangefinder is for measurement/vision assistance and does **not** replace local STM32 cliff/proximity/bumper safety.
- The optical brow/window must be validated for ~940 nm transmission and crosstalk; arbitrary dark plastic is not accepted without test.
- TF-Luna becomes the intentional backup if >4 m range or >50 Hz is required; TFmini-S only if a real longer-range requirement appears.



---

# ENGINEERING + MEMORY LOOPS


> Source file: `docs/ENGINEERING_METHOD_AND_MEMORY_LOOP.md`


# Engineering Method, Memory Loop, and AI Working Rules

## Core engineering loop
For every meaningful design change:
1. Restate requirement and measurable success criteria.
2. Research manufacturer datasheets, application notes, standards/guidance, and relevant engineering literature.
3. Generate at least one credible alternative.
4. Perform FMEA and identify single-point/common-cause failures.
5. Check tolerances, manufacturability, assembly and service access.
6. Check misuse, jams, power loss, unplugging, blocked sensors, and human-contact cases.
7. Define a test that can falsify the design assumption.
8. Re-check interfaces with every affected subsystem.
9. Run change-impact analysis on mass, COM, power, space, thermals, wiring, firmware and safety.
10. Check wear, replacement, supply-chain and future upgrade path.
11. Red-team the result and either approve conditionally, freeze, or redesign.

## Memory / inventory loop
Before answering a new project question or approving a change, review:
- current master spec;
- component matrix;
- current CAD parameters and validation;
- design decisions and decision log;
- unresolved freeze gates;
- electronics/power/safety interfaces;
- mechanical swept volumes/cables/service zones;
- AI/software boundaries;
- previous corrections and deprecated assumptions.

Ask: What did we already decide? What changed later? What has not been detailed enough? What dependency can the new change break? Which old subsystem has disappeared from the discussion but still matters?

## Conditional-decision rule
Never write only `CONDITIONAL`. Maintain:
- Candidate A (default),
- Candidate B/C (fallbacks),
- explicit switch condition,
- evidence/test needed to freeze the decision.

## AI/CAD rule
AI-generated visuals are not CAD authority. CAD authority comes from parametric source + real component envelopes + constraints + testable dimensions + validation scripts. Decorative image generation may guide industrial design only after it is checked against engineering geometry.

## Model collaboration rule
Use one model/agent as owner for a task, request an independent review only for high-risk/critical work, and avoid wasting credits by having multiple models duplicate routine work. Approved results must be promoted into this repository rather than living only in chat.


## Open-Source Harvest Loop
For each material subsystem decision: define the problem; search mature open-source robots and their issue trackers/troubleshooting docs; extract actual failure modes; adapt only the useful principle to ZI-E; check license before importing any code/CAD; add a verification test; then KEEP/REJECT. Current findings live in `OPEN_SOURCE_HARVEST_AUDIT.md`.



---

# OPEN-SOURCE HARVEST AUDIT


> Source file: `docs/OPEN_SOURCE_HARVEST_AUDIT.md`


# ZI-E Open-Source Harvest Audit — Pre-Purchase Pass 2026-08-23

## Rule
For every important ZI-E problem, inspect mature open-source robots and their issue trackers, not only showcase documentation. Extract the failure pattern, adapt the engineering principle to ZI-E, then verify it with simulation/bench tests before buying a large batch of parts.

This document records problems found in Reachy Mini/Reachy, LeRobot SO-101, OpenArm, Stretch, MoveIt/Nav2 and MuJoCo and the preventive changes now adopted by ZI-E.

| External failure/lesson | Evidence | ZI-E risk | Preventive change adopted now |
|---|---|---|---|
| Motor IDs/baud/configuration can be wrong or duplicated; calibration can fail | Reachy motor diagnosis; LeRobot SO-101 issues | Wrong joint moves on first power-up; arm/head damage | Commissioning mode: discover IDs, verify expected model/voltage/baud, one joint at a time, low torque/current, no automatic animation until calibration passes |
| A wrong motor position/orientation can create overload and plastic linkage damage | Reachy Mini issue #1184 | Head/arm mechanism damage during assembly | Add mechanical assembly index marks, keyed adapters where possible, low-power direction test, software joint plausibility check before normal torque |
| Cable slack inside a moving head can restrict motion and overload motors | Reachy motor diagnosis guide | Head pan/tilt/lift cable becomes an unintended motion stop | Defined cable loop, flex-rated wire, assembly slack gauge/mark, bend-radius envelope in CAD, full-range cable-cycle test |
| Gearbox backlash can cause oscillation/jitter near an unstable neutral pose | Reachy Mini troubleshooting | SC09 Pan/Tilt gaze jitter | Belt preload, configurable deadband/PID, gravity/spring bias where useful, CAD provision for output reference sensing; ST3036 remains precision fallback |
| Motors can overheat or enter overload protection | Reachy Mini troubleshooting; SO-101 reports | Long hold at arm/head loads damages servo or drops communication | Continuous temperature/current monitoring where available, time-at-load limits, dynamic torque/speed derating, no design based on stall torque |
| Over-voltage faults can break communication/control software if fault handling is coupled | Reachy Mini issue #589 | Servo fault crashes higher control path | Device I/O errors become fault states, never exceptions that kill safety loop; rail voltage checked against servo limits; regen/transient protection retained |
| Commands may be accepted even when torque is disabled | Reachy Mini issue #1306 listing | AI believes motion happened when robot was not armed | Command protocol separates ACCEPTED from EXECUTED/REJECTED; actual-state confirmation is mandatory for safety transitions |
| Multiple command sources can cause jerky motion | Reachy Mini debugging | AI/web/manual controllers fight each other | One command arbiter/lease per subsystem; daemon is sole high-level motion authority; STM32 remains final actuator gate |
| Serial/daisy-chain wiring creates cascading missing-device failures | Reachy motor diagnosis | One cable/servo disconnect hides downstream devices | Avoid physical dependency on servo pass-through where practical; use short parallel bus branches/hub harnesses, separate left/right/head buses, individually protected power branches |
| Payload changes cause arm sag if control model assumes empty end-effector | OpenArm discussion | Tool/object mass changes torque/stability | Tool metadata includes mass/COM; grasped-object estimate updates motion/stability limits; unknown object uses conservative limit |
| Added output bearings improved OpenArm payload | OpenArm release notes | Servo shaft/gearbox bears structural loads | Keep ZI-E policy: bearings/axles carry structural loads, motors provide torque only; verify one-sided joints for stiffness |
| CAD can contain features that are hard/impossible to manufacture | OpenArm hardware issue #15 | Nice CAD cannot be printed/CNC'd reliably | DFM review before freeze: tool radii, thread access, heat-set inserts, printer orientation, support access, replaceable wear parts |
| Collision checking needs simplified collision geometry and extensive sampling | MoveIt Setup Assistant | Detailed visual CAD either slow or misses self-collision rules | Separate visual meshes from low-poly collision meshes; generate high-density self-collision matrix; preserve padding for environment collision |
| Fast motion needs collision/velocity scaling near obstacles/singularities | MoveIt Servo | Planner-safe target can still be unsafe during execution | Runtime velocity scaling + local STM32 limits; continuous trajectory checks in simulation |
| Collision safety should stop/slow/limit based on zones and stale sensor data should stop | Nav2 Collision Monitor | Base keeps moving on stale proximity data; static footprint ignores arms | Dynamic stop/slow/approach envelope expands with speed, head height, arm extension/tool; stale critical range data => stop; add hysteresis/consecutive-cycle filtering |
| Diagnostic tools are necessary on a multi-subsystem robot | Stretch diagnostics/system-check approach | Debugging assembled robot becomes guesswork | `zie-system-check` planned: power, version, servo scan, sensors, limits, camera, audio, tools, BMS, network; diagnostic bundle/log export |
| Firmware/software compatibility needs explicit version management | Stretch firmware updater architecture | New firmware breaks older daemon/config | Hardware revision + protocol + ESP32 FW + STM32 FW + SDK version manifest and compatibility table; rollback path |
| Simulation mass/inertia inferred from cosmetic mesh can be wrong | MuJoCo modeling guidance | Digital twin looks right but dynamics are wrong | Explicit measured mass/COM/inertia per link; simple convex collision geometry; don't infer inertias from decorative shell |
| I2C RGB matrix is convenient but slower than video-oriented matrices | Adafruit IS31FL3741 docs | Belly matrix animation design exceeds bus capability | BLM limited to simple icons/text/ambient animation, dedicated buffered I2C, independent power/enable; no video requirement |

## Immediate architecture changes from this audit

1. **Safe commissioning state is mandatory.** First power-up cannot run normal animations or stow sequences until joint identity/direction/range checks pass.
2. **Every actuator command carries execution semantics.** `accepted`, `armed`, `executing`, `completed`, `rejected`, `faulted` are distinct.
3. **Single control arbiter.** Manual UI, AI, scripted behaviors and teleoperation cannot write joints independently at the same time.
4. **Bus fault containment.** Left arm, right arm, head and tools remain separate fault/power domains where practical. Avoid a cable topology where one first device is a physical gateway to all others.
5. **Dynamic payload model.** Tool and grasped-object mass/COM become part of stability and joint torque limits.
6. **Dynamic base safety envelope.** Stop/slow/approach distances depend on speed, arm extension, tool length, payload and head height.
7. **Diagnostic/commissioning software is a V1 requirement**, not an afterthought.
8. **Simulation uses explicit inertials and simplified collision meshes.** Visual CAD is never the collision/dynamics authority by itself.
9. **Belly Matrix is isolated from safety.** It is power-limited, noncritical, and cannot share a single point of failure with cliff sensing.

## Parts still intentionally conditional before purchase
- 3.5-inch ILI9488 touch panel: exact FPC pinout and touch-controller identity.
- Head Pan/Tilt SC09: jitter/backlash/temperature bench test; ST3036/HL2915 backups remain.
- Shoulder/Elbow actuator set: dummy-arm sustained-load and thermal test with real link masses.
- Wheel motors/tire: final mass, traction and braking-distance test.
- 4× downward cliff sensing: black/glass/mirror/tablecloth/sunlight matrix; mechanical fallback provision retained.
- P20/15 magnet: real pull-force vs gap/material plus temperature test.
- BQ77915 shunt/FET/fuse sizing: measured system current and transient test.
- Belly optical stack: off-state invisibility/readability/thermal coupon test.

## Reviewed sources
- Reachy Mini troubleshooting: https://github.com/pollen-robotics/reachy_mini/blob/main/docs/source/troubleshooting.md
- Reachy Mini motor diagnosis: https://github.com/pollen-robotics/reachy_mini/blob/main/docs/source/troubleshooting/motors_diagnosis.md
- Reachy Mini issue #1184: https://github.com/pollen-robotics/reachy_mini/issues/1184
- Reachy Mini issue #589: https://github.com/pollen-robotics/reachy_mini/issues/589
- LeRobot SO-101 docs: https://github.com/huggingface/lerobot/blob/main/docs/source/so101.mdx
- LeRobot issue #2819: https://github.com/huggingface/lerobot/issues/2819
- LeRobot issue #3193: https://github.com/huggingface/lerobot/issues/3193
- OpenArm: https://github.com/enactic/OpenArm
- OpenArm releases: https://github.com/enactic/openarm/releases
- OpenArm hardware manufacturing issue #15: https://github.com/enactic/openarm_hardware/issues/15
- Stretch diagnostics: https://github.com/hello-robot/stretch_diagnostics
- MoveIt Setup Assistant: https://moveit.picknik.ai/main/doc/examples/setup_assistant/setup_assistant_tutorial.html
- MoveIt URDF/SRDF: https://moveit.picknik.ai/main/doc/examples/urdf_srdf/urdf_srdf_tutorial.html
- Nav2 Collision Monitor: https://docs.nav2.org/configuration/packages/collision_monitor/configuring-collision-monitor-node.html
- MuJoCo XML reference: https://mujoco.readthedocs.io/en/stable/XMLreference.html
- Adafruit IS31FL3741: https://learn.adafruit.com/adafruit-is31fl3741



---

# LASER RANGEFINDER SPEC


> Source file: `docs/LASER_RANGEFINDER_SPEC.md`


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



---

# BELLY MATRIX SPEC


> Source file: `docs/BELLY_MATRIX_SPEC.md`


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



---

# PRE-PURCHASE GATE


> Source file: `docs/PRE_PURCHASE_GATE.md`


# ZI-E Pre-Purchase Gate

Do not bulk-order parts merely because a CAD envelope fits. Buy the minimum parts needed to retire the highest-risk uncertainties first.

## Phase A — cheap evidence before bulk purchase
1. One 3.5-inch display candidate + confirm exact pinout/touch IC.
2. One OV5640 AF module.
3. One VL53L1X rangefinder carrier/sample for head-brow optical and alignment tests.
4. One ESP32-S3 N16R8 development carrier and one STM32G0B1 test board/carrier.
5. One STS3215, one HL2915, one SC09, one N20 encoder motor.
6. One wheel motor + DRV8876 test channel.
7. One VL53L4CD breakout initially; surface tests before buying all 8 sensors.
8. One IS31FL3741 13×9 belly-matrix breakout + several optical-window coupons.
9. One P20/15 magnet sample (two if generic supplier quality is uncertain).

## Mandatory bench tests before batch order
- Servo ID/model/baud discovery and safe low-torque commissioning.
- Sustained rated-load temperature, backlash, repeatability, audible noise.
- Head cable loop through Pan/Tilt/Lift full travel and jam/power-loss behavior.
- Head laser rangefinder: known-distance accuracy, dark/daylight, black/glossy/glass targets, final cover-window coupons, crosstalk calibration, Pan/Tilt stability and camera-axis alignment.
- Arm dummy-load test using estimated link/tool masses.
- Wheel traction/braking/current on representative desk surfaces.
- Cliff sensor surface matrix: white, matte black, glossy black, wood, metal, mirror, clear glass, tablecloth, edge shapes, daylight, dirty sensor.
- Belly matrix optical coupon: hidden off-state, time/icon readability, daylight, current, heat, light bleed.
- Magnet pull force vs steel size/thickness/paint/gap and coil temperature.
- Power transient/regen and brownout fault injection.

## Bulk purchase condition
A part becomes `BUY-BATCH` only after its test result is stored in `/tests/results/` and the component matrix status is changed from `CONDITIONAL` to `VERIFIED`.



---

# OPEN ITEMS / FREEZE GATES


> Source file: `docs/OPEN_ITEMS_AND_FREEZE_GATES.md`


# Open Items / Freeze Gates

The project may continue, but the following cannot be silently treated as final:

1. Display: confirm exact selected FPC pinout, touch controller, connector, backlight and initialization details.
2. Head: measure complete head assembly mass/COM; re-size Tilt ratio and lift/tilt counterbalance from measurements.
3. Arms: build weighted dummy arm; verify rated-torque margins, backlash, stall/jam behavior, belt tension, thermal rise and endurance.
4. Telescope: verify lead-screw/nut wear, thrust bearing, stroke, wobble and dynamic cable life.
5. Wheels: choose final tire diameter/width/material and pulley ratio from traction, noise, scrub and braking tests.
6. Cliff: validate black/glass/mirror/sunlight/dirty-sensor cases; add different-principle mechanical fallback if needed.
7. Magnet: characterize pull force against representative parts and air gaps; characterize continuous/limited duty temperature.
8. Battery/BMS: measure actual worst normal/blocked currents; then freeze shunt, MOSFETs, fuse and wire/connector sizes.
9. Audio: A/B microphone ports and speaker chamber; verify AEC with motors and speaker simultaneously active.
10. Wiring: validate bend radii, strain relief, service loops, EMI and wear cycle counts.
11. Shell: apply final industrial-design surfaces only after all service/collision/thermal/sensor keep-outs pass.
12. Charging: V1 uses external 12.6V CC/CV; integrated USB-C PD is a future revision, not a hidden requirement.


## Belly Matrix / hidden HMI
- Optical dead-front coupon: off-state concealment vs lit readability.
- Sunlight/daylight visibility and color shift.
- Diffuser/baffle distance and light-leak test.
- Full-white/current/temperature stress.
- Dedicated non-safety bus and fail-off power gating.

## Commissioning and fault containment
- Actuator scan/model/baud/ID/direction/range before normal torque.
- Bus topology / cable fault injection: loss of one actuator/cable must degrade predictably.
- Command protocol must distinguish accepted from physically executed.
- Command arbiter prevents multiple simultaneous setpoint writers.
- Diagnostic bundle/system-check must exist before full integrated prototype testing.


## Head laser rangefinder freeze gate
- Verify VL53L1X prototype carrier and final sensor brow packaging.
- Select/measure IR-transmissive cover material around 940 nm.
- Minimize air gap/tilt and perform offset + crosstalk calibration.
- Calibrate camera-to-rangefinder optical axes and document the ranging point.
- Test known distances, dark/daylight, black/glossy/glass targets and contamination.
- Do not promote to BUY-BATCH until results are saved under `tests/results/`.



---

# CAD STATUS


> Source file: `docs/CAD_STATUS.md`


# Current CAD Status

The current CAD source of truth is `mechanical/cad/current/`.

## Snapshot
- Parametric source: `src/zie_cad.py`
- Validation: `src/validate_design.py`
- Rendering/export helper: `src/render_gltf.py`
- Four assembly states: ACTIVE, SLEEP, SHUTDOWN, CUTAWAY
- Individual STEP/STL exports for major structural/cosmetic parts
- Requirements traceability, FMEA, parameter data, bounding boxes, mass/COM, validation matrix and renders included.

## Automated validation snapshot
From `data/validation_summary.json`:
- 39 tests total
- 38 pass
- 1 restricted/verify case
- estimated mass ≈ 2448 g
- estimated active COM ≈ (-0.6, -2.4, 91.3) mm in CAD coordinates
- ACTIVE bounding box ≈ 248 mm × 173.86 mm × 282 mm
- SLEEP height ≈ 247 mm
- SHUTDOWN height ≈ 182 mm

The restricted case is both arms simultaneously carrying 250 g forward: static support margin is ~12.2 mm versus the 15 mm normal-driving target. Firmware therefore treats that combination as a restricted manipulation condition (base locked/very-low acceleration or lower payload).

## Interpretation
This is prototype engineering CAD, not production manufacturing freeze. Any low/medium-confidence mass, clearance, torque, acoustic, thermal, traction, magnetic-force or optical-edge assumption must be replaced by measured hardware data before release to manufacturing.

- v0.3 includes the hidden Belly Light Matrix and adds the dedicated head VL53L1X laser-rangefinder prototype envelope, IR-window rules and camera/rangefinder alignment gate.



---

# CAD RELEASE


> Source file: `mechanical/cad/current/docs/CAD_RELEASE.md`


# ZI-E CAD v0.3 — Prototype Release

## Release meaning

This is the first complete, parameterized mechanical/package CAD of the current ZI-E architecture. It is complete enough to print fit-test parts, simulate state envelopes, build subsystem rigs, and create the electronics PCB around real keep-outs.

It is **not yet a production freeze** because several dimensions depend on real-part measurement and physical tests. That distinction is deliberate engineering practice, not unfinished bookkeeping.

## Coordinate convention
- X = left/right
- Y = front (-) / rear (+)
- Z = up
- units = mm

## Generated system states
- `ZI-E_v0_3_ACTIVE.step`
- `ZI-E_v0_3_SLEEP.step`
- `ZI-E_v0_3_SHUTDOWN.step`
- `ZI-E_v0_3_CUTAWAY.step`
- matching glTF viewer exports

## Current generated envelopes

The automated assembly bounding-box output is stored in `data/assembly_bounding_boxes.json`. Current nominal design intent is approximately:
- Active height: ~282 mm.
- Main body/base envelope: ~184 × 142 mm before external wheel/arm envelope.
- Active full pose envelope: ~248 mm wide and ~174 mm front/rear including relaxed hands.
- Protected shutdown envelope: ~220 mm wide × ~151 mm deep × ~182 mm high.

These values are *outputs* of the component packing model, not aesthetic dimensions chosen first.

## Head
- 3.5 in landscape IPS-touch display envelope: 84.52 × 55.26 × 4 mm.
- OV5640 AF camera envelope reserved in a top-center sensor brow.
- Dual MEMS mic port reservations symmetric around the camera.
- Pan: SC09, 1:1 belt, structural hollow bearing ring.
- Tilt: SC09, ~2:1 belt, dual-side bearing yoke, COM-centered.
- Lift: N20 encoder motor + T5×2 lead screw + one precision rail/two carriage reference + secondary anti-rotation guide provision.
- Active / Sleep / Shutdown are generated from the same model.
- Shutdown top is flush with the body top plane; no latch is used.
- Three structural shutdown seats are included.
- Hollow pan center and dynamic cable keep-out are reserved.

## Body / structure
- Soft tapered old-ZI-E visual direction, not the later industrial humanoid poster direction.
- Cosmetic shell is non-load-bearing.
- Lower structural chassis + left/right shoulder towers + rear bridge surround a protected central Head Bay.
- Side arm-stow pockets are cut through the cosmetic shell, with structure behind them.
- Rear service cover has no essential wiring attached.
- Separate lower-rear battery access door.
- Hidden lower-front speaker chamber/vents.
- ESP32 antenna keep-out reserved at upper rear nonmetallic shell.

## Mobility
- **Two powered wheels only**, differential drive.
- 25D/54L encoder gearmotor class drives an independently bearing-supported axle through a short belt.
- Manual wheel tuck is modeled about an upper-inner longitudinal hinge; stowed tires clear the desk and the robot rests on four fixed feet.
- Hard-stop/lock geometry is reserved.
- One hidden front-center POM ball caster is the third active contact point.
- Passive front/rear anti-tip skids are modeled.

## Desk-edge / collision safety
- Four fixed downward VL53L4CD-class optical windows near the four base corners.
- Four horizontal ToF reservations for local obstacle sensing.
- Four compliant bumper segments as a sensing-independent physical fallback.
- Downward sensor layout remains a physical-test freeze gate for glass, black/glossy surfaces, mirror, sunlight and contamination.

## Battery / electronics package
- Low central 3S2P M35A cassette; six cells modeled at Molicel maximum cell diameter/height.
- Rear-extracting cassette rails, front hard stop and latch/handle provision.
- Rear service-board envelopes for multimedia, safety and power modules.
- BQ7791500-class pack-protection architecture is reserved electrically; shunt/FET/fuse values remain measurement gates.
- Separate power and signal/service zones maintained.

## Arms
Per arm:
1. two bearing-supported shoulder axes, STS3215-class actuators near the body;
2. proximal HL2915 elbow actuator with short belt to the elbow axle;
3. keyed one-stage telescopic forearm;
4. N20 + T5×2 extension drive with end-sensor/thrust-bearing provisions;
5. limited forearm roll, SC09;
6. wrist pitch, SC09 with reduction;
7. manual quick-swap tool interface.

The CAD contains Active, Reach, Desk and Stow kinematic key poses for validation. The current shell pockets clear the defined distal-arm geometry in all four key poses.

## Quick-swap tools
- 3-lug bayonet concept.
- independent pull/twist safety collar architecture.
- asymmetric key.
- 8-contact pattern: 2× V+, 2× GND, differential data A/B, Tool ID, AUX.
- electrical contacts carry no structural load.
- gripper and magnetic-tool envelope prototypes included.

## Manufacturing assumptions
Prototype-oriented starting values are defined in `data/parameters.json` and are intentionally adjustable. The authoritative model is parameterized so printer/material calibration does not require manually redrawing the robot.

## Automated validation result
See `VALIDATION_REPORT.md` and `data/validation_matrix.csv`.

One load case is intentionally marked restricted: **both arms fully forward with 250 g payload on each hand** reduces the calculated static primary-support margin below the normal-driving target. The design response is not to hide that result: firmware must lock/slow the base or reduce simultaneous payload/reach in that state.

## Production-freeze gates
The following must be completed before “final manufacturing CAD” is claimed:
1. Measure the real display and obtain exact FPC/touch-controller documentation.
2. Print fit coupons and calibrate moving/static/insert clearances for the actual printer/material.
3. Weigh the assembled head and measure its COM; then freeze Tilt reduction/counterbalance.
4. Build one dummy arm and perform horizontal-hold, transient, backlash and thermal tests using rated—not stall—torque.
5. Cycle-test head lift, telescope, wheel fold and tool bayonet for wear/jam/cable fatigue.
6. Validate cliff sensing on the required surface matrix.
7. Measure real robot current waveforms; then freeze battery shunt/FET/fuse/eFuse settings.
8. Run speaker/microphone A/B acoustic tests with motors/display active.
9. Run thermal tests with the shell installed and in shutdown.
10. Replace low-confidence mass estimates with measured part masses and rerun stability.

- Added dedicated head laser rangefinder packaging and camera/rangefinder alignment gate.



---

# CAD VALIDATION


> Source file: `mechanical/cad/current/docs/VALIDATION_REPORT.md`


# ZI-E CAD v0.3 — Automated Geometry / Engineering Validation

**Result:** 38/39 checks pass. Failed checks are design restrictions or freeze gates, not silently ignored.

Estimated mass: **2.45 kg**; active estimated COM: **(-0.6, -2.4, 91.3) mm**.

| Check | Value | Target | Result |
|---|---:|---|---|
| Head bay side clearance per side | 3.5 | >= 1.5 mm | PASS |
| Head bay depth clearance per side | 4.0 | >= 2.0 mm | PASS |
| Display side bezel each side | 15.740000000000002 | >= 10 mm | PASS |
| Display vertical margin total | 26.740000000000002 | >= 20 mm | PASS |
| Camera-to-lidar horizontal packaging gap | 2.25 | >= 2 mm | PASS |
| Lidar carrier inside head width | 17.25 | >= 10 mm side structure | PASS |
| Belly prototype PCB inside reserved keepout width | 28.700000000000003 | >= 20 mm spare | PASS |
| Belly prototype PCB inside reserved keepout height | 15.0 | >= 10 mm spare | PASS |
| Belly optical window thickness | 1.2 | 0.8-1.5 mm prototype range | PASS |
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
| Estimated total mass | 2448 | informational | PASS |
| Baseline static support margin | 20.4 | >= 18 mm | PASS |
| One-arm 250g forward static margin | 19.8 | >= 15 mm | PASS |
| Two-arm 2x250g forward static margin | 12.1 | >= 15 mm for normal driving | RESTRICT / VERIFY |
| Protected worst commanded rail budget | 15.5 | < 20 A cell ceiling | PASS |

## Important interpretation

- The two-arm 2×250 g forward case is intentionally allowed to fail the normal-driving margin. Firmware must treat that combination as a restricted manipulation state (base stationary or much lower payload/acceleration).
- Torque calculations use **rated torque**, never stall torque, and include a simple belt-loss factor. Physical thermal/load tests remain the authority.
- Mass values tagged low/medium confidence must be replaced with measured values after parts are printed/bought.
- Collision tests cover the defined key poses. Continuous trajectory collision checking belongs in the simulation/firmware phase.


---

# WEB / DATASHEET SOURCES


> Source file: `mechanical/cad/current/docs/WEB_RESEARCH_SOURCES.md`


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



---

# V0.3 RELEASE NOTES


> Source file: `docs/RELEASE_v0.3.md`


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



---

# ROADMAP


> Source file: `docs/ROADMAP.md`


# ZI-E Roadmap

## Phase A — Concept Design
Status: substantially complete.

- visual personality;
- operating poses;
- modularity direction;
- basic system architecture;
- service philosophy.

## Phase B — Design Freeze v0.1
Next.

- overall mechanical envelope;
- proportions → real dimensions;
- head motion/retraction envelope;
- arm motion/stow envelope;
- wheel/tuck envelope;
- battery/service envelope;
- internal zones;
- hard interfaces.

## Phase C — Component Verification
Review one component at a time.

For each:
- role;
- real datasheet;
- dimensions;
- voltage/current;
- interfaces;
- thermal/mechanical constraints;
- availability/cost;
- alternatives;
- KEEP / REPLACE / UNDECIDED.

## Phase D — Electrical Architecture
Only after component direction is sufficiently stable.

- power tree;
- charging;
- battery protection;
- current budget;
- buses/interfaces;
- controller resource budget;
- pinout;
- connectors;
- safety.

## Phase E — Digital Prototype / Simulation
Before full purchase.

- CAD fit;
- kinematic checks;
- arm/retraction collision checks;
- center of gravity estimates;
- electrical simulation where useful;
- firmware mocks;
- laptop brain protocol;
- virtual command/state testing.

## Phase F — Breadboard / Bench Bring-up

- power;
- controller;
- communications;
- one actuator at a time;
- sensors;
- audio;
- display/camera;
- integrated safety.

## Phase G — Mechanical Prototype

- print/build;
- assemble modules;
- validate motion;
- iterate clearances;
- verify serviceability.

## Phase H — AI Brain Integration

- wireless protocol;
- speech;
- vision;
- memory/database;
- behavior;
- safe intent layer.

## Phase I — Used Phone Migration

After laptop-based behavior/database is stable:
- edge compute feasibility;
- local database sync;
- phone↔robot protocol;
- offline behavior;
- cloud fallback;
- power/thermal strategy.


## Immediate pre-purchase risk-retirement step
Run the minimum-parts bench campaign in `PRE_PURCHASE_GATE.md` before ordering batches. Prioritize actuator commissioning/thermal tests, cliff surface tests, head cable loop, wheel traction/braking, battery transients, magnet pull/thermal, and belly optical coupons.



---

# GITHUB HANDOFF


> Source file: `docs/GITHUB_HANDOFF_CURRENT.md`


# GitHub Handoff — Current Snapshot

This repository is intended to be the portable source of truth for ZI-E across ChatGPT accounts and other AI tools.

## Start here
1. `README.md`
2. `docs/ZI-E_MASTER_SPEC.md`
3. `docs/COMPONENT_MASTER_MATRIX.md`
4. `docs/CURRENT_STATE.md`
5. `docs/ENGINEERING_METHOD_AND_MEMORY_LOOP.md`
6. `mechanical/cad/current/docs/VALIDATION_REPORT.md`
7. `docs/OPEN_ITEMS_AND_FREEZE_GATES.md`
8. `docs/LASER_RANGEFINDER_SPEC.md`
9. `docs/RELEASE_v0.3.md`

## Authority order
1. Current explicit user correction/decision
2. Current master spec/current CAD + measured tests
3. Approved design-decision log
4. Manufacturer datasheet/application notes
5. Research notes
6. Legacy files (reference only)
7. Decorative/generated concept art (visual reference only)

## GitHub upload
Upload the contents of the `ZI-E/` folder as the repository root. Large binary CAD files are currently below GitHub's single-file hard limit, but Git LFS is recommended long-term if the CAD history grows substantially.

## Branch policy
Use `main` as the current stable design branch. Use feature branches for major CAD/electronics/firmware experiments once normal Git workflow is available.



---

# RELEASE FILE MANIFEST


- `.gitignore`
- `AGENTS.md`
- `PROJECT_CONTEXT.md`
- `README.md`
- `REPOSITORY_MAP.md`
- `REPO_FILE_LIST.txt`
- `SHA256SUMS.txt`
- `assets/branding/ZI-E_AI_Time_Robot_Emblem.png`
- `assets/concepts/ZI-E_Concept_Sheet_v0.1.png`
- `assets/concepts/ZI-E_Concept_Sheet_v0.2_hidden_camera_rgb.png`
- `assets/concepts/ZI-E_current_visual_reference.png`
- `assets/concepts/ZI-E_early_concept_board.png`
- `assets/deprecated/generated_posters/README.md`
- `assets/deprecated/generated_posters/a_detailed_engineering_design_poster_infographic.png`
- `assets/deprecated/generated_posters/a_detailed_infographic_engineering_concept_board.png`
- `docs/AI_WORKFLOW.md`
- `docs/ARCHITECTURE.md`
- `docs/BELLY_MATRIX_SPEC.md`
- `docs/CAD_STATUS.md`
- `docs/CHAT_DECISION_LOG.md`
- `docs/COMPONENT_MASTER_MATRIX.md`
- `docs/CONVERSATION_ARCHIVE_POLICY.md`
- `docs/CURRENT_STATE.md`
- `docs/DESIGN_DECISIONS.md`
- `docs/ENGINEERING_METHOD_AND_MEMORY_LOOP.md`
- `docs/GITHUB_BINARY_FILES.md`
- `docs/GITHUB_HANDOFF_CURRENT.md`
- `docs/LASER_RANGEFINDER_SPEC.md`
- `docs/LEGACY_AUDIT.md`
- `docs/MIGRATION_HANDOFF.md`
- `docs/OPEN_ITEMS_AND_FREEZE_GATES.md`
- `docs/OPEN_SOURCE_HARVEST_AUDIT.md`
- `docs/PRE_PURCHASE_GATE.md`
- `docs/RELEASE_v0.3.md`
- `docs/ROADMAP.md`
- `docs/ZI-E_MASTER_SPEC.md`
- `docs/conversations/2026-08-23_continuation_summary.md`
- `docs/conversations/2026-08-23_full_project_chat.md`
- `docs/conversations/2026-08-23_full_project_chat_original.md`
- `docs/conversations/README.md`
- `electronics/README.md`
- `firmware/README.md`
- `firmware/secrets.example.h`
- `legacy/README.md`
- `legacy/firmware/WALL_E_AI_Brain_v4_1_Faces.ino`
- `legacy/firmware/WALL_E_Faces_Engine.ino`
- `legacy/guides/README_WALLE_AI_v4.md`
- `legacy/guides/WALL-E_Corrected_Master_Guide_3.pdf`
- `legacy/guides/WALLE_Super_Master_Guide_Arms_Updated_REV2_FINAL_2.pdf`
- `legacy/guides/WALL_E_Assembly_Guide.md`
- `legacy/guides/walle_super_guide_2.pdf`
- `mechanical/README.md`
- `mechanical/cad/archive/v0.1/ZI-E_CAD_v0_1.scad`
- `mechanical/cad/archive/v0.1/ZI-E_CAD_v0_1.stl`
- `mechanical/cad/archive/v0.1/ZI-E_CAD_v0_1_NOTES.md`
- `mechanical/cad/archive/v0.1/ZI-E_CAD_v0_1_cutaway.png`
- `mechanical/cad/archive/v0.1/ZI-E_CAD_v0_1_exterior.png`
- `mechanical/cad/archive/v0.1/ZI-E_CAD_v0_1_preview.png`
- `mechanical/cad/archive/v0.1/ZI-E_CAD_v0_1_skeleton.png`
- `mechanical/cad/archive/v1.0-lite/data/FMEA_CAD.csv`
- `mechanical/cad/archive/v1.0-lite/data/assembly_bounding_boxes.json`
- `mechanical/cad/archive/v1.0-lite/data/component_envelopes.csv`
- `mechanical/cad/archive/v1.0-lite/data/mass_com.csv`
- `mechanical/cad/archive/v1.0-lite/data/mass_com_summary.json`
- `mechanical/cad/archive/v1.0-lite/data/parameters.json`
- `mechanical/cad/archive/v1.0-lite/data/requirements_traceability.csv`
- `mechanical/cad/archive/v1.0-lite/data/validation_console.json`
- `mechanical/cad/archive/v1.0-lite/data/validation_matrix.csv`
- `mechanical/cad/archive/v1.0-lite/data/validation_summary.json`
- `mechanical/cad/archive/v1.0-lite/docs/AI_CAD_WORKFLOW.md`
- `mechanical/cad/archive/v1.0-lite/docs/CAD_RELEASE.md`
- `mechanical/cad/archive/v1.0-lite/docs/ENGINEERING_LOOPS.md`
- `mechanical/cad/archive/v1.0-lite/docs/VALIDATION_REPORT.md`
- `mechanical/cad/archive/v1.0-lite/renders/ZI-E_v1_0_ACTIVE_front.png`
- `mechanical/cad/archive/v1.0-lite/renders/ZI-E_v1_0_ACTIVE_iso.png`
- `mechanical/cad/archive/v1.0-lite/renders/ZI-E_v1_0_CUTAWAY_front.png`
- `mechanical/cad/archive/v1.0-lite/renders/ZI-E_v1_0_CUTAWAY_iso.png`
- `mechanical/cad/archive/v1.0-lite/renders/ZI-E_v1_0_SHUTDOWN_front.png`
- `mechanical/cad/archive/v1.0-lite/renders/ZI-E_v1_0_SHUTDOWN_iso.png`
- `mechanical/cad/archive/v1.0-lite/renders/ZI-E_v1_0_SLEEP_front.png`
- `mechanical/cad/archive/v1.0-lite/renders/ZI-E_v1_0_SLEEP_iso.png`
- `mechanical/cad/archive/v1.0-lite/renders/test.png`
- `mechanical/cad/archive/v1.0-lite/src/__pycache__/zie_cad.cpython-313.pyc`
- `mechanical/cad/archive/v1.0-lite/src/render_gltf.py`
- `mechanical/cad/archive/v1.0-lite/src/validate_design.py`
- `mechanical/cad/archive/v1.0-lite/src/zie_cad.py`
- `mechanical/cad/archive/v1.1/README.md`
- `mechanical/cad/archive/v1.1/SHA256SUMS.txt`
- `mechanical/cad/archive/v1.1/cad/ZI-E_v1_1_ACTIVE.bin`
- `mechanical/cad/archive/v1.1/cad/ZI-E_v1_1_ACTIVE.gltf`
- `mechanical/cad/archive/v1.1/cad/ZI-E_v1_1_ACTIVE.step`
- `mechanical/cad/archive/v1.1/cad/ZI-E_v1_1_CUTAWAY.bin`
- `mechanical/cad/archive/v1.1/cad/ZI-E_v1_1_CUTAWAY.gltf`
- `mechanical/cad/archive/v1.1/cad/ZI-E_v1_1_CUTAWAY.step`
- `mechanical/cad/archive/v1.1/cad/ZI-E_v1_1_SHUTDOWN.bin`
- `mechanical/cad/archive/v1.1/cad/ZI-E_v1_1_SHUTDOWN.gltf`
- `mechanical/cad/archive/v1.1/cad/ZI-E_v1_1_SHUTDOWN.step`
- `mechanical/cad/archive/v1.1/cad/ZI-E_v1_1_SLEEP.bin`
- `mechanical/cad/archive/v1.1/cad/ZI-E_v1_1_SLEEP.gltf`
- `mechanical/cad/archive/v1.1/cad/ZI-E_v1_1_SLEEP.step`
- `mechanical/cad/archive/v1.1/cad/anti_tip_skid_1.step`
- `mechanical/cad/archive/v1.1/cad/anti_tip_skid_2.step`
- `mechanical/cad/archive/v1.1/cad/arm_L_forearm_shell.step`
- `mechanical/cad/archive/v1.1/cad/arm_L_inner_shell.step`
- `mechanical/cad/archive/v1.1/cad/arm_L_upper_shell.step`
- `mechanical/cad/archive/v1.1/cad/arm_R_forearm_shell.step`
- `mechanical/cad/archive/v1.1/cad/arm_R_inner_shell.step`
- `mechanical/cad/archive/v1.1/cad/arm_R_upper_shell.step`
- `mechanical/cad/archive/v1.1/cad/base_shell.step`
- `mechanical/cad/archive/v1.1/cad/battery_cassette.step`
- `mechanical/cad/archive/v1.1/cad/battery_door.step`
- `mechanical/cad/archive/v1.1/cad/belly_deadfront_window_REFERENCE.step`
- `mechanical/cad/archive/v1.1/cad/belly_matrix_baffle.step`
- `mechanical/cad/archive/v1.1/cad/bumper_segment_1.step`
- `mechanical/cad/archive/v1.1/cad/bumper_segment_2.step`
- `mechanical/cad/archive/v1.1/cad/bumper_segment_3.step`
- `mechanical/cad/archive/v1.1/cad/bumper_segment_4.step`
- `mechanical/cad/archive/v1.1/cad/forearm_inner_keyed.step`
- `mechanical/cad/archive/v1.1/cad/forearm_outer_keyed.step`
- `mechanical/cad/archive/v1.1/cad/gripper_finger_dist.step`
- `mechanical/cad/archive/v1.1/cad/gripper_finger_prox.step`
- `mechanical/cad/archive/v1.1/cad/gripper_palm.step`
- `mechanical/cad/archive/v1.1/cad/gripper_tool_body.step`
- `mechanical/cad/archive/v1.1/cad/head_bezel.step`
- `mechanical/cad/archive/v1.1/cad/head_carriage.step`
- `mechanical/cad/archive/v1.1/cad/head_rear_hatch.step`
- `mechanical/cad/archive/v1.1/cad/head_shell.step`
- `mechanical/cad/archive/v1.1/cad/lower_chassis.step`
- `mechanical/cad/archive/v1.1/cad/magnet_tool_housing.step`
- `mechanical/cad/archive/v1.1/cad/magnet_tool_housing_float.step`
- `mechanical/cad/archive/v1.1/cad/quick_swap_female.step`
- `mechanical/cad/archive/v1.1/cad/quick_swap_male.step`
- `mechanical/cad/archive/v1.1/cad/quick_swap_safety_collar.step`
- `mechanical/cad/archive/v1.1/cad/rear_bridge.step`
- `mechanical/cad/archive/v1.1/cad/rear_service_cover.step`
- `mechanical/cad/archive/v1.1/cad/shoulder_bearing_yoke.step`
- `mechanical/cad/archive/v1.1/cad/shoulder_tower_L.step`
- `mechanical/cad/archive/v1.1/cad/shoulder_tower_R.step`
- `mechanical/cad/archive/v1.1/cad/shutdown_foot_1.step`
- `mechanical/cad/archive/v1.1/cad/shutdown_foot_2.step`
- `mechanical/cad/archive/v1.1/cad/shutdown_foot_3.step`
- `mechanical/cad/archive/v1.1/cad/shutdown_foot_4.step`
- `mechanical/cad/archive/v1.1/cad/speaker_chamber.step`
- `mechanical/cad/archive/v1.1/cad/tilt_yoke.step`
- `mechanical/cad/archive/v1.1/cad/torso_shell.step`
- `mechanical/cad/archive/v1.1/cad/upper_arm_housing.step`
- `mechanical/cad/archive/v1.1/cad/wheel_belt_cover_L.step`
- `mechanical/cad/archive/v1.1/cad/wheel_belt_cover_R.step`
- `mechanical/cad/archive/v1.1/cad/wheel_fairing_L.step`
- `mechanical/cad/archive/v1.1/cad/wheel_fairing_R.step`
- `mechanical/cad/archive/v1.1/cad/wheel_pod_prototype.step`
- `mechanical/cad/archive/v1.1/cad/wrist_pitch_yoke.step`
- `mechanical/cad/archive/v1.1/data/FMEA_CAD.csv`
- `mechanical/cad/archive/v1.1/data/assembly_bounding_boxes.json`
- `mechanical/cad/archive/v1.1/data/component_envelopes.csv`
- `mechanical/cad/archive/v1.1/data/mass_com.csv`
- `mechanical/cad/archive/v1.1/data/mass_com_summary.json`
- `mechanical/cad/archive/v1.1/data/mesh_quality.csv`
- `mechanical/cad/archive/v1.1/data/mesh_quality_summary.json`
- `mechanical/cad/archive/v1.1/data/parameters.json`
- `mechanical/cad/archive/v1.1/data/requirements_traceability.csv`
- `mechanical/cad/archive/v1.1/data/validation_console.json`
- `mechanical/cad/archive/v1.1/data/validation_matrix.csv`
- `mechanical/cad/archive/v1.1/data/validation_summary.json`
- `mechanical/cad/archive/v1.1/docs/AI_CAD_WORKFLOW.md`
- `mechanical/cad/archive/v1.1/docs/BELLY_MATRIX_DESIGN.md`
- `mechanical/cad/archive/v1.1/docs/CAD_RELEASE.md`
- `mechanical/cad/archive/v1.1/docs/ENGINEERING_LOOPS.md`
- `mechanical/cad/archive/v1.1/docs/FREEZE_GATES.md`
- `mechanical/cad/archive/v1.1/docs/OPEN_SOURCE_HARVEST_AUDIT.md`
- `mechanical/cad/archive/v1.1/docs/PRE_PURCHASE_GATE.md`
- `mechanical/cad/archive/v1.1/docs/VALIDATION_REPORT.md`
- `mechanical/cad/archive/v1.1/docs/WEB_RESEARCH_SOURCES.md`
- `mechanical/cad/archive/v1.1/renders/ZI-E_v1_1_ACTIVE_front.png`
- `mechanical/cad/archive/v1.1/renders/ZI-E_v1_1_ACTIVE_iso.png`
- `mechanical/cad/archive/v1.1/renders/ZI-E_v1_1_CUTAWAY_front.png`
- `mechanical/cad/archive/v1.1/renders/ZI-E_v1_1_CUTAWAY_iso.png`
- `mechanical/cad/archive/v1.1/renders/ZI-E_v1_1_SHUTDOWN_iso.png`
- `mechanical/cad/archive/v1.1/renders/ZI-E_v1_1_SLEEP_iso.png`
- `mechanical/cad/archive/v1.1/renders/test.png`
- `mechanical/cad/archive/v1.1/src/render_gltf.py`
- `mechanical/cad/archive/v1.1/src/validate_design.py`
- `mechanical/cad/archive/v1.1/src/zie_cad.py`
- `mechanical/cad/archive/v1.1/stl/anti_tip_skid_1.stl`
- `mechanical/cad/archive/v1.1/stl/anti_tip_skid_2.stl`
- `mechanical/cad/archive/v1.1/stl/arm_L_forearm_shell.stl`
- `mechanical/cad/archive/v1.1/stl/arm_L_inner_shell.stl`
- `mechanical/cad/archive/v1.1/stl/arm_L_upper_shell.stl`
- `mechanical/cad/archive/v1.1/stl/arm_R_forearm_shell.stl`
- `mechanical/cad/archive/v1.1/stl/arm_R_inner_shell.stl`
- `mechanical/cad/archive/v1.1/stl/arm_R_upper_shell.stl`
- `mechanical/cad/archive/v1.1/stl/base_shell.stl`
- `mechanical/cad/archive/v1.1/stl/battery_cassette.stl`
- `mechanical/cad/archive/v1.1/stl/battery_door.stl`
- `mechanical/cad/archive/v1.1/stl/belly_deadfront_window_REFERENCE.stl`
- `mechanical/cad/archive/v1.1/stl/belly_matrix_baffle.stl`
- `mechanical/cad/archive/v1.1/stl/bumper_segment_1.stl`
- `mechanical/cad/archive/v1.1/stl/bumper_segment_2.stl`
- `mechanical/cad/archive/v1.1/stl/bumper_segment_3.stl`
- `mechanical/cad/archive/v1.1/stl/bumper_segment_4.stl`
- `mechanical/cad/archive/v1.1/stl/forearm_inner_keyed.stl`
- `mechanical/cad/archive/v1.1/stl/forearm_outer_keyed.stl`
- `mechanical/cad/archive/v1.1/stl/gripper_finger_dist.stl`
- `mechanical/cad/archive/v1.1/stl/gripper_finger_prox.stl`
- `mechanical/cad/archive/v1.1/stl/gripper_palm.stl`
- `mechanical/cad/archive/v1.1/stl/gripper_tool_body.stl`
- `mechanical/cad/archive/v1.1/stl/head_bezel.stl`
- `mechanical/cad/archive/v1.1/stl/head_carriage.stl`
- `mechanical/cad/archive/v1.1/stl/head_rear_hatch.stl`
- `mechanical/cad/archive/v1.1/stl/head_shell.stl`
- `mechanical/cad/archive/v1.1/stl/lower_chassis.stl`
- `mechanical/cad/archive/v1.1/stl/magnet_tool_housing.stl`
- `mechanical/cad/archive/v1.1/stl/magnet_tool_housing_float.stl`
- `mechanical/cad/archive/v1.1/stl/quick_swap_female.stl`
- `mechanical/cad/archive/v1.1/stl/quick_swap_male.stl`
- `mechanical/cad/archive/v1.1/stl/quick_swap_safety_collar.stl`
- `mechanical/cad/archive/v1.1/stl/rear_bridge.stl`
- `mechanical/cad/archive/v1.1/stl/rear_service_cover.stl`
- `mechanical/cad/archive/v1.1/stl/shoulder_bearing_yoke.stl`
- `mechanical/cad/archive/v1.1/stl/shoulder_tower_L.stl`
- `mechanical/cad/archive/v1.1/stl/shoulder_tower_R.stl`
- `mechanical/cad/archive/v1.1/stl/shutdown_foot_1.stl`
- `mechanical/cad/archive/v1.1/stl/shutdown_foot_2.stl`
- `mechanical/cad/archive/v1.1/stl/shutdown_foot_3.stl`
- `mechanical/cad/archive/v1.1/stl/shutdown_foot_4.stl`
- `mechanical/cad/archive/v1.1/stl/speaker_chamber.stl`
- `mechanical/cad/archive/v1.1/stl/tilt_yoke.stl`
- `mechanical/cad/archive/v1.1/stl/torso_shell.stl`
- `mechanical/cad/archive/v1.1/stl/upper_arm_housing.stl`
- `mechanical/cad/archive/v1.1/stl/wheel_belt_cover_L.stl`
- `mechanical/cad/archive/v1.1/stl/wheel_belt_cover_R.stl`
- `mechanical/cad/archive/v1.1/stl/wheel_fairing_L.stl`
- `mechanical/cad/archive/v1.1/stl/wheel_fairing_R.stl`
- `mechanical/cad/archive/v1.1/stl/wheel_pod_prototype.stl`
- `mechanical/cad/archive/v1.1/stl/wrist_pitch_yoke.stl`
- `mechanical/cad/current/README.md`
- `mechanical/cad/current/SHA256SUMS.txt`
- `mechanical/cad/current/cad/ZI-E_v0_3_ACTIVE.bin`
- `mechanical/cad/current/cad/ZI-E_v0_3_ACTIVE.gltf`
- `mechanical/cad/current/cad/ZI-E_v0_3_ACTIVE.step`
- `mechanical/cad/current/cad/ZI-E_v0_3_CUTAWAY.bin`
- `mechanical/cad/current/cad/ZI-E_v0_3_CUTAWAY.gltf`
- `mechanical/cad/current/cad/ZI-E_v0_3_CUTAWAY.step`
- `mechanical/cad/current/cad/ZI-E_v0_3_SHUTDOWN.bin`
- `mechanical/cad/current/cad/ZI-E_v0_3_SHUTDOWN.gltf`
- `mechanical/cad/current/cad/ZI-E_v0_3_SHUTDOWN.step`
- `mechanical/cad/current/cad/ZI-E_v0_3_SLEEP.bin`
- `mechanical/cad/current/cad/ZI-E_v0_3_SLEEP.gltf`
- `mechanical/cad/current/cad/ZI-E_v0_3_SLEEP.step`
- `mechanical/cad/current/cad/anti_tip_skid_1.step`
- `mechanical/cad/current/cad/anti_tip_skid_2.step`
- `mechanical/cad/current/cad/arm_L_forearm_shell.step`
- `mechanical/cad/current/cad/arm_L_inner_shell.step`
- `mechanical/cad/current/cad/arm_L_upper_shell.step`
- `mechanical/cad/current/cad/arm_R_forearm_shell.step`
- `mechanical/cad/current/cad/arm_R_inner_shell.step`
- `mechanical/cad/current/cad/arm_R_upper_shell.step`
- `mechanical/cad/current/cad/base_shell.step`
- `mechanical/cad/current/cad/battery_cassette.step`
- `mechanical/cad/current/cad/battery_door.step`
- `mechanical/cad/current/cad/belly_deadfront_window_REFERENCE.step`
- `mechanical/cad/current/cad/belly_matrix_baffle.step`
- `mechanical/cad/current/cad/bumper_segment_1.step`
- `mechanical/cad/current/cad/bumper_segment_2.step`
- `mechanical/cad/current/cad/bumper_segment_3.step`
- `mechanical/cad/current/cad/bumper_segment_4.step`
- `mechanical/cad/current/cad/forearm_inner_keyed.step`
- `mechanical/cad/current/cad/forearm_outer_keyed.step`
- `mechanical/cad/current/cad/gripper_finger_dist.step`
- `mechanical/cad/current/cad/gripper_finger_prox.step`
- `mechanical/cad/current/cad/gripper_palm.step`
- `mechanical/cad/current/cad/gripper_tool_body.step`
- `mechanical/cad/current/cad/head_bezel.step`
- `mechanical/cad/current/cad/head_carriage.step`
- `mechanical/cad/current/cad/head_rear_hatch.step`
- `mechanical/cad/current/cad/head_shell.step`
- `mechanical/cad/current/cad/lower_chassis.step`
- `mechanical/cad/current/cad/magnet_tool_housing.step`
- `mechanical/cad/current/cad/magnet_tool_housing_float.step`
- `mechanical/cad/current/cad/quick_swap_female.step`
- `mechanical/cad/current/cad/quick_swap_male.step`
- `mechanical/cad/current/cad/quick_swap_safety_collar.step`
- `mechanical/cad/current/cad/rear_bridge.step`
- `mechanical/cad/current/cad/rear_service_cover.step`
- `mechanical/cad/current/cad/shoulder_bearing_yoke.step`
- `mechanical/cad/current/cad/shoulder_tower_L.step`
- `mechanical/cad/current/cad/shoulder_tower_R.step`
- `mechanical/cad/current/cad/shutdown_foot_1.step`
- `mechanical/cad/current/cad/shutdown_foot_2.step`
- `mechanical/cad/current/cad/shutdown_foot_3.step`
- `mechanical/cad/current/cad/shutdown_foot_4.step`
- `mechanical/cad/current/cad/speaker_chamber.step`
- `mechanical/cad/current/cad/tilt_yoke.step`
- `mechanical/cad/current/cad/torso_shell.step`
- `mechanical/cad/current/cad/upper_arm_housing.step`
- `mechanical/cad/current/cad/wheel_belt_cover_L.step`
- `mechanical/cad/current/cad/wheel_belt_cover_R.step`
- `mechanical/cad/current/cad/wheel_fairing_L.step`
- `mechanical/cad/current/cad/wheel_fairing_R.step`
- `mechanical/cad/current/cad/wheel_pod_prototype.step`
- `mechanical/cad/current/cad/wrist_pitch_yoke.step`
- `mechanical/cad/current/data/FMEA_CAD.csv`
- `mechanical/cad/current/data/assembly_bounding_boxes.json`
- `mechanical/cad/current/data/component_envelopes.csv`
- `mechanical/cad/current/data/mass_com.csv`
- `mechanical/cad/current/data/mass_com_summary.json`
- `mechanical/cad/current/data/mesh_quality.csv`
- `mechanical/cad/current/data/mesh_quality_summary.json`
- `mechanical/cad/current/data/parameters.json`
- `mechanical/cad/current/data/requirements_traceability.csv`
- `mechanical/cad/current/data/validation_matrix.csv`
- `mechanical/cad/current/data/validation_summary.json`
- `mechanical/cad/current/docs/AI_CAD_WORKFLOW.md`
- `mechanical/cad/current/docs/BELLY_MATRIX_DESIGN.md`
- `mechanical/cad/current/docs/CAD_RELEASE.md`
- `mechanical/cad/current/docs/ENGINEERING_LOOPS.md`
- `mechanical/cad/current/docs/FREEZE_GATES.md`
- `mechanical/cad/current/docs/LASER_RANGEFINDER_DESIGN.md`
- `mechanical/cad/current/docs/OPEN_SOURCE_HARVEST_AUDIT.md`
- `mechanical/cad/current/docs/PRE_PURCHASE_GATE.md`
- `mechanical/cad/current/docs/VALIDATION_REPORT.md`
- `mechanical/cad/current/docs/WEB_RESEARCH_SOURCES.md`
- `mechanical/cad/current/renders/ZI-E_v0_3_ACTIVE_front.png`
- `mechanical/cad/current/renders/ZI-E_v0_3_ACTIVE_iso.png`
- `mechanical/cad/current/renders/ZI-E_v0_3_CUTAWAY_front.png`
- `mechanical/cad/current/renders/ZI-E_v0_3_CUTAWAY_iso.png`
- `mechanical/cad/current/renders/ZI-E_v0_3_SHUTDOWN_iso.png`
- `mechanical/cad/current/renders/ZI-E_v0_3_SLEEP_iso.png`
- `mechanical/cad/current/renders/test.png`
- `mechanical/cad/current/src/render_gltf.py`
- `mechanical/cad/current/src/validate_design.py`
- `mechanical/cad/current/src/zie_cad.py`
- `mechanical/cad/current/stl/anti_tip_skid_1.stl`
- `mechanical/cad/current/stl/anti_tip_skid_2.stl`
- `mechanical/cad/current/stl/arm_L_forearm_shell.stl`
- `mechanical/cad/current/stl/arm_L_inner_shell.stl`
- `mechanical/cad/current/stl/arm_L_upper_shell.stl`
- `mechanical/cad/current/stl/arm_R_forearm_shell.stl`
- `mechanical/cad/current/stl/arm_R_inner_shell.stl`
- `mechanical/cad/current/stl/arm_R_upper_shell.stl`
- `mechanical/cad/current/stl/base_shell.stl`
- `mechanical/cad/current/stl/battery_cassette.stl`
- `mechanical/cad/current/stl/battery_door.stl`
- `mechanical/cad/current/stl/belly_deadfront_window_REFERENCE.stl`
- `mechanical/cad/current/stl/belly_matrix_baffle.stl`
- `mechanical/cad/current/stl/bumper_segment_1.stl`
- `mechanical/cad/current/stl/bumper_segment_2.stl`
- `mechanical/cad/current/stl/bumper_segment_3.stl`
- `mechanical/cad/current/stl/bumper_segment_4.stl`
- `mechanical/cad/current/stl/forearm_inner_keyed.stl`
- `mechanical/cad/current/stl/forearm_outer_keyed.stl`
- `mechanical/cad/current/stl/gripper_finger_dist.stl`
- `mechanical/cad/current/stl/gripper_finger_prox.stl`
- `mechanical/cad/current/stl/gripper_palm.stl`
- `mechanical/cad/current/stl/gripper_tool_body.stl`
- `mechanical/cad/current/stl/head_bezel.stl`
- `mechanical/cad/current/stl/head_carriage.stl`
- `mechanical/cad/current/stl/head_rear_hatch.stl`
- `mechanical/cad/current/stl/head_shell.stl`
- `mechanical/cad/current/stl/lower_chassis.stl`
- `mechanical/cad/current/stl/magnet_tool_housing.stl`
- `mechanical/cad/current/stl/magnet_tool_housing_float.stl`
- `mechanical/cad/current/stl/quick_swap_female.stl`
- `mechanical/cad/current/stl/quick_swap_male.stl`
- `mechanical/cad/current/stl/quick_swap_safety_collar.stl`
- `mechanical/cad/current/stl/rear_bridge.stl`
- `mechanical/cad/current/stl/rear_service_cover.stl`
- `mechanical/cad/current/stl/shoulder_bearing_yoke.stl`
- `mechanical/cad/current/stl/shoulder_tower_L.stl`
- `mechanical/cad/current/stl/shoulder_tower_R.stl`
- `mechanical/cad/current/stl/shutdown_foot_1.stl`
- `mechanical/cad/current/stl/shutdown_foot_2.stl`
- `mechanical/cad/current/stl/shutdown_foot_3.stl`
- `mechanical/cad/current/stl/shutdown_foot_4.stl`
- `mechanical/cad/current/stl/speaker_chamber.stl`
- `mechanical/cad/current/stl/tilt_yoke.stl`
- `mechanical/cad/current/stl/torso_shell.stl`
- `mechanical/cad/current/stl/upper_arm_housing.stl`
- `mechanical/cad/current/stl/wheel_belt_cover_L.stl`
- `mechanical/cad/current/stl/wheel_belt_cover_R.stl`
- `mechanical/cad/current/stl/wheel_fairing_L.stl`
- `mechanical/cad/current/stl/wheel_fairing_R.stl`
- `mechanical/cad/current/stl/wheel_pod_prototype.stl`
- `mechanical/cad/current/stl/wrist_pitch_yoke.stl`
- `simulation/README.md`
- `software/README.md`
- `tests/README.md`