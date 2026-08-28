# Hardware Source-of-Truth Reconciliation — Prototype P1

Status: canonical repository reconciliation as of 2026-08-28. This is a design
inventory, not proof of purchase, receipt, fit, electrical suitability, or
commissioning. The future purchased-parts sheet, label photos, datasheets, and measured
worksheets supersede candidate assumptions for the received article.

## Status vocabulary

- `VERIFIED`: physically identified/measured on the received article. No component in
  this matrix currently qualifies.
- `DOCUMENTED_NOT_PHYSICALLY_VERIFIED`: repository decision/candidate supported by a
  cited source, but not reconciled to received hardware.
- `VERIFY_ON_ARRIVAL`: exact variant, interface, envelope, rating, or behavior must be
  checked before use/CAD freeze.
- `REPLACE`: repository decision rejects the item. None is newly asserted here.
- `UNDECIDED`: selection or implementation is still open.
- `WAITING_FOR_VERIFIED_INPUTS`: an architecture placeholder exists, but the listed
  physical evidence is required before parameters or implementation may be frozen.

`Power` contains only repository-documented class information. `TBD` is intentional.
Slots and drivers are semantic mappings; they confer no raw hardware authority.

## Canonical component/module matrix

| Canonical component/module | Role; owner | Documented interface / power | Hardware Profile slot; driver/extension class | CAD dependency; safety | Status | Evidence; physical verification required |
|---|---|---|---|---|---|---|
| 3.5-inch 480x320 IPS touch display, ILI9488 candidate | Face/display; ESP32-S3 | Exact FPC/touch interface TBD; power TBD | `display.face`; ESP32 display embedded module | Head bezel, thickness, FPC route; non-safety | `VERIFY_ON_ARRIVAL` | Component matrix + v0.3 envelope; exact panel/IC, active area, thickness, holes/adhesive, connector/FPC orientation, voltage/current |
| Adafruit IS31FL3741 13x9 prototype matrix | Belly light prototype; ESP32-S3 | Driver class documented; exact wiring/power TBD | `rgb.belly`; ESP32 presentation embedded module | 80x54x12 reserved keepout; cosmetic, must fail dark | `VERIFY_ON_ARRIVAL` | Component matrix/Belly spec; exact PCB, connector, current/thermal/optics |
| Secret-until-lit belly insert/baffle | Optical/mechanical; passive | PC/PMMA candidate; no electrical interface | `presentation.belly_optics`; data/mechanical | Torso window/baffle; non-safety but thermal/fault containment relevant | `VERIFY_ON_ARRIVAL` | CAD/Belly spec; material, thickness, transmission, light bleed, temperature, fit |
| OV5640 5 MP autofocus camera candidate | Vision; ESP32-S3 | Exact module/FPC interface and power TBD | `camera.head`; ESP32 camera embedded module | Sensor brow, FPC loop/window; non-safety | `VERIFY_ON_ARRIVAL` | Component matrix/CAD envelope; exact module/revision, lens, board size, connector, focus/clearance/FOV |
| VL53L1X head rangefinder | Non-safety forward range; ESP32-S3 | Sensor interface documented elsewhere as I2C; power/pins TBD | `range.head`; ESP32 range embedded module | IR window, camera alignment, carrier; non-safety | `VERIFY_ON_ARRIVAL` | Laser spec/CAD; exact carrier vs bare sensor, window transmission/crosstalk, FOV/alignment, known-distance behavior |
| ESP32-S3-WROOM-1-N16R8 candidate | Multimedia/presence/semantic host; ESP32-S3 | TWAI and multimedia interfaces; board/carrier/power TBD | `controller.multimedia`; protected host composition, not plugin raw access | ESP32 zone, RF/USB/service access; authority boundary relevant | `VERIFY_ON_ARRIVAL` | Component matrix/vendor-derived envelope; exact module/board/revision, dimensions, antenna keepout, USB/boot, voltage/current |
| STM32G0B1RET6 candidate/custom carrier reservation | Local safety/motion; STM32 | FDCAN-capable MCU; carrier/pins/power TBD | `controller.safety`; built-in protected safety module | STM32 zone, debug/service, actuator gating; safety-critical | `VERIFY_ON_ARRIVAL` | Component matrix/CAD carrier reservation; exact MCU/board/revision, clocks, debug, dimensions, pins, voltage/current |
| ESP32–STM32 Classical CAN/TWAI link | Bounded semantic link; ESP32/STM32 | Classical CAN only; bitrate/IDs/framing/transceiver TBD | `link.controller`; protected controller-link service | Harness/topology/termination; safety-critical liveness | `WAITING_FOR_VERIFIED_INPUTS` | Phase 2B2 analysis; all inputs in `PHASE2B2_INPUT_GATE.md` |
| LSM6DSOX IMU candidate | Inertial sensing; STM32 for safety-relevant state unless later approved otherwise | Interface/power TBD | `imu.body`; protected sensor driver if used for safety | Board placement/alignment; safety relevance depends on use | `VERIFY_ON_ARRIVAL` | Component matrix; exact breakout/variant, interface, orientation, noise, dimensions, voltage/current |
| VL53L4CD downward cliff sensors, four planned | Local cliff sensing; STM32 | Interface/power/addressing TBD | `cliff.downward.*`; built-in protected safety sensor driver | Lower windows/angles/ground clearance; safety-critical | `VERIFY_ON_ARRIVAL` | Component matrix; count/variant, FOV, addresses, window, surface/sunlight/occlusion behavior |
| VL53L4CD horizontal proximity, four planned/conditional | Local proximity; STM32 | Interface/power/addressing TBD | `proximity.horizontal.*`; protected sensor driver if safety-gating | Bumper/lower shell windows; safety-relevant | `UNDECIDED` | Component matrix permits reduction to two; Product Owner decision plus physical coverage testing required |
| D2F-L-class bumper switches, four planned | Contact collision input; STM32 | Discrete interface/power TBD | `bumper.contact.*`; built-in protected safety input | Bumper segments, preload/travel; safety-critical | `VERIFY_ON_ARRIVAL` | Component matrix; exact switch/lever, force/travel, mounting, wiring, debounce, fault response |
| 25SG-370CA-78-EN-class 12 V encoded wheel motors | Differential drive; STM32 | Encoded motor; exact encoder/driver wiring TBD; 12 V class documented | `drive.left/right`; built-in protected drive driver | Wheel pods, shaft/mount, belt/fairing; safety-critical | `VERIFY_ON_ARRIVAL` | Component matrix/CAD envelope; exact label/ratio, shaft, holes, encoder, no-load/load current, speed, thermal |
| DRV8876 protected H-bridge class, two | Wheel power stages; STM32 | Exact carrier/interface TBD; sizing TBD | `drive.power.left/right`; built-in protected drive driver | Power board/heatsinking; safety-critical | `VERIFY_ON_ARRIVAL` | Component matrix; exact part/carrier, current/thermal/protection, fault behavior |
| 64 mm-class wheel/tire and folding pod | Traction/support; passive + STM32 drive | Mechanical; no final interface | `drive.wheel.*`; physical drive assembly | Diameter/tread/fold lock/support polygon; safety-critical | `VERIFY_ON_ARRIVAL` | v0.3 CAD parameter/model only; measured diameter, width, traction, braking, runout, lock loads |
| POM ball-caster cartridge candidate | Passive third contact | Mechanical | `support.caster`; passive | Base support/COM; safety-relevant stability | `UNDECIDED` | Component matrix/CAD 25.4 mm model; exact unit, height, friction, mounting, surface behavior |
| Molicel M35A-class 18650 cells, 3S2P architecture | Energy storage; passive/protected power | 3S2P class; exact pack implementation TBD | `battery.main`; built-in protected power service | Battery cassette/COM/fire containment; safety-critical | `VERIFY_ON_ARRIVAL` | Component matrix/CAD; cell provenance/labels, lot, dimensions/mass, pack construction, voltage/internal behavior |
| BQ7791500-class pack protection | BMS/protection; protected power/STM32 monitoring as designed | Exact variant/schema TBD | `power.pack_protection`; built-in protected safety power module | Power board/thermal/service; safety-critical | `UNDECIDED` | Component matrix marks implementation conditional; exact variant, schematic, thresholds, shunt/FET/fuse evidence |
| Certified external 12.6 V ~2 A CC/CV charger class | V1 charging; external/passive | 12.6 V class documented; connector TBD | `power.charger_external`; no plugin control | Charge connector/service; safety-critical | `VERIFY_ON_ARRIVAL` | Component matrix; certification/provenance, polarity, connector, measured output, pack compatibility |
| AP62300-class general buck regulators | Logic/multimedia rails; power board | Exact rails/sizing TBD | `power.logic_rail.*`; protected power driver | Power board/thermal; safety-relevant isolation | `UNDECIDED` | Component matrix only class-level; exact part, rail requirements, load/inrush/thermal measurements |
| TPS62162-class safety 3.3 V supply | Safety logic rail; STM32/protected power | Exact implementation TBD | `power.safety_3v3`; built-in protected power driver | Safety board/power separation; safety-critical | `UNDECIDED` | Component matrix class-level; exact design, load/transient/thermal/fault isolation |
| TPS25947/eFuse-class arm/head protection | Branch current limiting; STM32/protected power | Exact implementation/limits TBD | `power.actuator_branch.*`; built-in protected power driver | Power board/harness; safety-critical | `UNDECIDED` | Component matrix class-level; exact part, limits, fault response, measured branch loads |
| Feetech STS3215-C018 shoulder servos, two per arm planned | Shoulder motion; STM32 | Servo bus/voltage/IDs not frozen | `arm.*.shoulder.*`; protected actuator driver | Shoulder towers/yokes; safety-critical motion | `VERIFY_ON_ARRIVAL` | Component matrix/CAD envelope; exact model/voltage, identity, bus/baud/ID, horn/spline, current/thermal/backlash |
| Feetech HL-2915-C001 elbow servo + ~2:1 candidate reduction | Elbow motion; STM32 | Servo bus/voltage not frozen | `arm.*.elbow`; protected actuator driver | Arm housing/belt geometry; safety-critical motion | `VERIFY_ON_ARRIVAL` | Component matrix/CAD estimate; exact servo, ratio/pulleys, current/thermal/load/backlash |
| Waveshare SC09-class servos | Forearm roll, wrist pitch, pan, tilt, gripper candidates; STM32 | Interface/voltage/IDs not frozen | corresponding `arm.*`, `head.pan/tilt`, `gripper.*`; protected actuator drivers | Multiple mechanism envelopes; safety-critical motion | `VERIFY_ON_ARRIVAL` | Component matrix/CAD envelope; exact count/model, voltage, identity, bus, spline, range, current/thermal/jitter |
| N20 encoder motor + T5x2 lead mechanism | Telescope and head lift candidates; STM32 | Exact motor/encoder/driver/end sensing TBD | `arm.*.telescope`, `head.lift`; protected actuator drivers | Rail/lead screw/cable loop; safety-critical motion | `VERIFY_ON_ARRIVAL` | Component matrix/CAD model; exact motor/gear ratio, screw/nut, travel, current, end sensors, backlash, jam/thermal |
| Adaptive tendon gripper mechanism | Grasping; STM32 | SC09 candidate; tendon details TBD | `gripper.left/right`; protected actuator driver | Palm/fingers/tendon routes; motion safety | `VERIFY_ON_ARRIVAL` | Component matrix/current CAD concept; actuator, tendon, force, travel, pinch risk, retention/cycle life |
| P20/15 12 V electromagnet-class tool | Magnetic tool; STM32/tool controller | 12 V class; switching/sensing/connector TBD | `tool.magnet.*`; protected tool semantic driver | Tool housing/quick swap/thermal; safety-relevant dropped-object risk | `VERIFY_ON_ARRIVAL` | Component matrix/CAD; exact magnet, pull vs gap/material, current/temperature, residual force, retention |
| STM32C011-class tool MCU | Tool-local control; STM32-owned domain | UART/ADC/PWM/watchdog class; exact board TBD | `tool.controller.*`; protected embedded tool module | Tool volume/connector; safety-relevant | `UNDECIDED` | Component matrix class-level; exact MCU/board, power, firmware/debug, dimensions |
| Independent RS-485 per wrist | Tool data link; STM32 | Physical transceiver/connector/baud TBD | `tool.link.left/right`; protected tool-link service | Wrist connector/cable loop; safety/fault-containment relevant | `WAITING_FOR_VERIFIED_INPUTS` | Component matrix architecture only; exact transceiver, wiring, termination, connector, fault behavior |
| DMM-4026 I2S-class microphones, two | Audio capture; ESP32-S3 | I2S class; exact variant/power TBD | `audio.input.*`; ESP32 audio embedded module | Head ports, acoustic isolation/cable; non-safety | `VERIFY_ON_ARRIVAL` | Component matrix; exact units, dimensions, port/orientation, sensitivity/noise, interface/voltage |
| PUI AS04004PO-R-class speaker | Audio output; ESP32-S3 | Amp connection/power TBD | `audio.output`; ESP32 audio embedded module | Speaker chamber/vent; non-safety with thermal/acoustic considerations | `VERIFY_ON_ARRIVAL` | Component matrix/CAD envelope; exact speaker, dimensions, impedance/power, excursion, clearance |
| MAX98357A-class I2S amplifier | Speaker drive; ESP32-S3 | I2S class; exact carrier/power TBD | `audio.amplifier`; ESP32 audio embedded module | Audio zone/thermal/noise; non-safety | `VERIFY_ON_ARRIVAL` | Component matrix; exact board, pins, gain, voltage/current, thermal/noise |
| Head/arm/wheel hard stops, shutdown feet, brackets, fasteners, spacers | Structure/restraint; passive | Mechanical; final sizes TBD | physical assembly, no executable extension | All modules; safety/serviceability | `VERIFY_ON_ARRIVAL` | v0.3 CAD geometry only; material, print/process, thread, length, insert fit, load, clearance |

## Named alternatives and substitute-only candidates

These names remain visible because the current component matrix names them. They are
not inventory, approved equivalents, or permission to substitute silently. Every one
is `UNDECIDED` and requires Product Owner disposition plus the same identity, ratings,
fit, safety, driver, and Hardware Profile review as a primary candidate.

| Named alternative/candidate | Possible role; owner | Mapping if selected | Evidence still required |
|---|---|---|---|
| EastRising/BuyDisplay documented panel or equivalent | Face display; ESP32-S3 | `display.face`; ESP32 display module | Exact maker/model, panel/touch IC, FPC, voltage/current, dimensions and bezel fit |
| Wider custom IS31FL3741-class matrix | Belly light; ESP32-S3 | `rgb.belly`; ESP32 presentation module | Exact design, optical result, power/current, thermal and envelope |
| OV2640 module | Camera; ESP32-S3 | `camera.head`; ESP32 camera module | Exact module/revision, interface, image suitability, power and physical fit |
| TF-Luna or TFmini-S | Head range alternative; ownership must be re-approved | `range.head`; protected semantic range service | Exact model/interface/power, FOV/window, update behavior, fit and safety role |
| Same-class ESP32-S3 PSRAM variant | Multimedia controller; ESP32-S3 | `controller.multimedia` | Exact suffix/revision, pins/resources, memory, board/carrier and RF/USB envelope |
| Same-family STM32 equivalent | Safety controller; STM32 | `controller.safety` | Exact MCU/board, I/O/timers/FDCAN/clocks, safety analysis and carrier fit |
| UART development fallback | Development-only controller link; ESP32/STM32 | No production Hardware Profile substitution | Explicit test-only scope; it cannot satisfy or bypass the Phase 2B2 production link gate |
| BMI270 | IMU; STM32 if safety-relevant | `imu.body`; protected sensor driver | Exact carrier/interface, orientation, performance evidence, voltage/current and fit |
| Mechanical cliff feelers | Supplemental cliff input; STM32 | `cliff.downward.*`; protected safety input | Mechanism, coverage, failure behavior, durability, mounting and commissioning evidence |
| D2HW sealed-class switch | Bumper; STM32 | `bumper.contact.*`; protected safety input | Exact part/lever, force/travel, mounting, wiring and fault response |
| Equivalent 25 mm encoded gearmotor | Wheel drive; STM32 | `drive.left/right`; protected drive driver | Exact identity, envelope/shaft/encoder, voltage, load current, thermal and motion tests |
| Equivalent protected H-bridge | Wheel power stage; STM32 | `drive.power.left/right`; protected drive driver | Exact part/carrier, ratings, protection/fault behavior and thermal evidence |
| Pololu #2692 or custom caster | Passive support | `support.caster` | Exact article/design, height, mounting, friction, stability and surface behavior |
| Trusted equivalent 18650 | Energy storage; protected power | `battery.main` | Provenance, exact cell, pack-current/energy/safety review, dimensions and mass |
| USB-C PD charging concept | Future charger; protected/external power | No P1 substitution without new decision | Full electrical/safety architecture; explicitly optional/future, not P1 inventory |
| Equivalent regulator, isolated safety supply, or protected switch | Power conversion/protection | Corresponding protected `power.*` slot | Exact circuit/part, ratings, isolation/fault behavior, load/transient and thermal evidence |
| STS3215 elbow fallback; ST3036 or HL2915 motion fallback; stronger actuator | Arm/head/gripper motion; STM32 | Existing protected semantic actuator slot only | Exact model/voltage/bus, geometry, load/current/thermal/backlash and safety tests |
| Faster N20, changed lead, or justified stepper | Telescope/head-lift mechanism; STM32 | Existing protected linear-motion slot | Exact motor/transmission, end sensing, current/thermal, speed, jam behavior and fit |
| Larger magnet or EPM | Tool; STM32-owned domain | `tool.magnet.*`; protected tool driver | Exact identity, switching/sensing, force vs gap, current/thermal, retention and fit |
| Similar tool MCU | Tool-local controller; STM32-owned domain | `tool.controller.*` | Exact MCU/board, UART/ADC/PWM/watchdog resources, power, firmware/debug and fit |
| T5848 or four-microphone concept | Audio capture; ESP32-S3 | `audio.input.*`; ESP32 audio module | Exact unit/count, interface, acoustics, processing resources, voltage/current and fit |
| AS02404PO speaker | Audio output; ESP32-S3 | `audio.output`; ESP32 audio module | Exact impedance/power, acoustic suitability, dimensions and chamber fit |
| Equivalent I2S class-D amplifier | Audio output stage; ESP32-S3 | `audio.amplifier`; ESP32 audio module | Exact carrier/pins/gain, voltage/current, thermal/noise and fit |

## Explicit conflict and ambiguity register

1. `mechanical/cad/archive/**` is historical reference only. It cannot override
   `mechanical/cad/current` or current docs.
2. Current CAD uses v0.3 envelope assumptions (including display, camera, actuators,
   boards, battery and wheels). These are parametric packaging inputs, not proof that
   the purchased variant matches.
3. The STM32 CAD object is a custom-carrier reservation, not a selected final PCB.
4. Horizontal VL53L4CD count remains four versus conditional reduction to two; no
   silent choice is made.
5. Caster implementation, BMS variant, regulator/eFuse implementations, tool MCU, CAN
   transceivers, and RS-485 physical layer remain undecided.
6. CAD's prototype VL53L1X carrier and smaller final bare-sensor envelope are distinct;
   neither may silently substitute for the received module.
7. Backup actuators (STS3215/ST3036/HL2915 alternatives) are switch conditions, not
   purchased inventory or equivalent approvals.
8. CAD validation values for torque, speed, rail budget, mass, COM, and stability are
   estimates/sensitivity results. They do not verify physical performance.
9. A stale v0.3 README count (37/36) conflicted with the current validator/summary
   (39/38); this stage aligns the README to the current executable/output evidence.

## Software/hardware authority mapping

Physical identity is recorded by label/serial or explicit provisional local identity,
then bound to exactly one logical slot and controller in the Hardware Profile. ESP32
embedded modules own multimedia/presence; STM32 built-in protected modules own cliff,
bumper, power, drive, arms, head motion, gripper and tool safety. Host/plugins submit
semantic intent only. No mapping above grants GPIO/PWM/register/bus-frame/raw-actuator
access to the App, AI, provider, pack, or plugin.
