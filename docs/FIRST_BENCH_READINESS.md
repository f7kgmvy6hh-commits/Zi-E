# First Real Bench Readiness — 2026-08-30

Status: `EVIDENCE_COLLECTION_ONLY`. This report combines the 31-line procurement
inventory snapshot with the current reviewed evidence overlay. It authorizes no first power, powered subsystem
test, final wiring, production pin assignment, Profile binding, commissioning pass, or
robot authority.

## Safe work now

- Photograph received articles, both PCB sides, exact labels, connectors, pin legends,
  and associated packaging/order references.
- Measure unpowered dimensions, mounting patterns, connector pitch and orientation.
- HW-017 is physically present and in active bench use. Limit it to continuity,
  resistance and low-voltage DC checks appropriate to its evidenced condition/ranges;
  do not rely on unverified CAT markings or use it for mains work.
- Test `HW-016` data capability on a known low-risk, non-robot USB device.
- Record every observation with inventory ID, date, instrument/method and photo.

Independent spare-board/MAX9814 tests already completed are recorded evidence, not
blanket authorization to repeat or expand powered work. This report authorizes no new
powered action or integrated robot power.

## Priority evidence closure

### P0 — HW-010 5 V / 3 A seller-described power board

`DO NOT POWER OR CONNECT`.

Provide front/back high-resolution photos; all IC/transformer/regulator markings;
input/output terminal labels; exact order page; manufacturer schematic/datasheet;
documented input type/range; output voltage/tolerance; DC polarity or AC line/neutral/
earth identity as applicable; isolation/non-isolation evidence; continuous/peak current
and thermal conditions; short/over-current/over-voltage/thermal protection; required
upstream fuse/eFuse; connector pitch/type/rating; and proposed unpowered continuity/
isolation plus current-limited unloaded-output measurement procedures.

Do not infer any value from seller photos. Do not attach robot loads during initial
characterization. `HW-033` XL4015E remains an unowned candidate, not another available
regulator.

### P0 — HW-002 ordered ESP32-S3 camera board

`BLOCKED UNTIL ARRIVAL`. On arrival provide front/back photos; PCB model/revision;
ESP32 module marking; OV5640/sensor marking; camera FPC and orientation; USB connectors
and interface chips; exact order page; and board-vendor schematic/pinout. Compare every
seller GPIO against the delivered board before selecting an example. Seller Camera Pin
Map, GPIO Allocation, Full Wiring Map and Firmware Pin Defines remain provisional.

Only after review: USB-only boot/recovery/log evidence with peripherals absent, then a
camera-only example using the reviewed physical pin map. Production pins remain open.

### P0 — HW-017 multimeter

Physical presence and bounded low-voltage bench use are observed. Record exact
model/range/fuse/category, probe/jack, manual, and condition evidence before a
higher-risk measurement. Do not infer mains or CAT authorization.

### P1 — received ESP32-side peripheral candidates

Ownership remains unassigned until review.

| ID | Item | Evidence required before independent powered test | Conditional isolated test |
|---|---|---|---|
| HW-001 | inventory spare ESP32-S3-N16R8 board | explicitly link the photographed inventory unit to its physical PCB/module markings | BLOCKED_PENDING_EXPLICIT_PHYSICAL_IDENTITY_LINK; do not infer it is the tested HW678 unit |
| HW-003 | ILI9488 display/touch | both sides, PCB revision, display and touch ICs, every pin, VCC/logic documentation, touch/microSD schematic | current-limited display-only; touch separate |
| HW-004 | PCM5102(A) DAC | both sides, IC/regulator/jumpers, every power/I2S/output pin, VIN/logic and schematic | DAC-only I2S into safe measurement/load path |
| HW-005 | PAM8403 amplifier | both sides, IC/terminals, supply documentation, continuity proof speaker negatives are not ground | current-limited amplifier plus one known speaker; never ground/join BTL outputs |
| HW-006 | MAX9814 microphone | independent power/bias/ADC/acoustic/gain test PASS; preserve 1215–1219 mV bias and 393–767 mV peak-to-peak evidence | integrated audio NOT_COMMISSIONED; temporary GPIO4 is not production |
| HW-007 | verified VL53L0X identity | 0x29 and register 0xC0 = 0xEE identify silicon; ranging remains invalid/unstable and solder/electrical condition suspect | QUARANTINED; do not power in this pass; rework/replace; GPIO5/GPIO6 temporary only |
| HW-008 | WS2812-family breakouts | 5V/DI/GND input and 5V/DO/GND output identified; approximately 330 Ω resistor measured; independent 3.3 V RED/GREEN/BLUE/OFF sequence passed | DI/RGB bench PASS via temporary GPIO7; final 5 V and integrated commissioning NOT_VERIFIED; never use HW678 5Vin |
| HW-009 | 8 Ω speakers | markings, dimensions/mounting, DC resistance per speaker | one speaker only after amplifier evidence is reviewed |

## Provisional wiring and DO NOT CONNECT

Verified physical GPIO/pin mappings: none. Final harness and production pin freeze:
blocked. The workbook maps are engineering drafts only.

Still required: module VIN and logic levels; exact pin legends; connector identity,
pitch, mating and orientation; polarity and return paths; isolation; shared-ground
suitability; current budget/inrush; USB data capability; switch continuity; and
reviewed protection/fuse/eFuse strategy.

Do not expand or repeat active-peripheral tests without a reviewed next procedure,
energize `HW-010`, repower quarantined HW-007, use HW678 5Vin as a 5 V source,
create a common-ground/final harness, or connect battery, motion, STM32, or CAN paths.

## Blocked domains

- Power/first power: `BLOCKED_UNVERIFIED_POWER_HARDWARE`; unauthorized.
- ESP32: generic software build and spare-board independent bench evidence PASS;
  production HW-002 lane remains blocked because the board has not arrived.
- Inventory identity: the tested SPARE-HW678 board is not assigned to HW-001 unless a
  reviewed photo/order-to-unit link explicitly proves that mapping.
- Camera: `BLOCKED_UNTIL_ARRIVAL_AND_REVIEW`.
- Wiring: `PROVISIONAL_ENGINEERING_PLAN_ONLY`.
- STM32/motion: `BLOCKED_MISSING_REQUIRED_HARDWARE_AND_EVIDENCE`.
- Controller link/CAN: no transceiver, controller clock, topology, bitrate, traffic,
  timing or physical-link evidence. Phase2B2 remains `WAITING_FOR_VERIFIED_INPUTS`.
- Physical commissioning: zero gates passed; autonomous motion remains blocked;
  robot authority remains `NONE`.
