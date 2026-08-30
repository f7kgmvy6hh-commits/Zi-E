# First Real Bench Readiness — 2026-08-30

Status: `EVIDENCE_COLLECTION_ONLY`. This report uses the active 31-line inventory from
`Zi-E_Hardware_Master_Inventory.xlsx`. It authorizes no first power, powered subsystem
test, final wiring, production pin assignment, Profile binding, commissioning pass, or
robot authority.

## Safe work now

- Photograph received articles, both PCB sides, exact labels, connectors, pin legends,
  and associated packaging/order references.
- Measure unpowered dimensions, mounting patterns, connector pitch and orientation.
- Physically match and inspect `HW-017` before using it. Then limit it to continuity,
  resistance and low-voltage DC checks appropriate to its reviewed condition/ranges;
  do not rely on unverified CAT markings or use it for mains work.
- Test `HW-016` data capability on a known low-risk, non-robot USB device.
- Record every observation with inventory ID, date, instrument/method and photo.

No currently received active module is approved for power. The conditional tests below
become candidates only after their own identity, supply, logic and connector evidence
is reviewed; this does not authorize integrated robot power.

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

Provide actual meter front/back photos; exact model/range/fuse/category labels; probe
and jack close-ups; receipt-to-unit match; exact manual; physical condition and fuse/
battery check. Until matched, it cannot supply project measurement evidence.

### P1 — received ESP32-side peripheral candidates

Ownership remains unassigned until review.

| ID | Item | Evidence required before independent powered test | Conditional isolated test |
|---|---|---|---|
| HW-001 | spare ESP32-S3 board | front/back, board/revision/module, USB chips/connectors, exact order page, vendor schematic/pinout | USB-only boot/log/flash; no peripherals |
| HW-003 | ILI9488 display/touch | both sides, PCB revision, display and touch ICs, every pin, VCC/logic documentation, touch/microSD schematic | current-limited display-only; touch separate |
| HW-004 | PCM5102(A) DAC | both sides, IC/regulator/jumpers, every power/I2S/output pin, VIN/logic and schematic | DAC-only I2S into safe measurement/load path |
| HW-005 | PAM8403 amplifier | both sides, IC/terminals, supply documentation, continuity proof speaker negatives are not ground | current-limited amplifier plus one known speaker; never ground/join BTL outputs |
| HW-006 | MAX9814 microphone | both sides, IC/GAIN/AR/power/output labels, VIN documentation, output-bias procedure/result | bounded microphone/ADC test with no assumed GPIO |
| HW-007 | VL53L0X-style module | both sides, sensor/breakout and pins, VIN, regulator/level-shifter evidence, KEEP/REPLACE decision versus VL53L1X | independent I2C identity/ranging; does not satisfy head-range commissioning |
| HW-008 | WS2812/WS2811 breakouts | both sides, LED/driver and DI/DO/power labels, supply documentation, KEEP/REPLACE decision versus belly matrix | one-breakout current-limited test with reviewed ground/logic strategy |
| HW-009 | 8 Ω speakers | markings, dimensions/mounting, DC resistance per speaker | one speaker only after amplifier evidence is reviewed |

## Provisional wiring and DO NOT CONNECT

Verified physical GPIO/pin mappings: none. Final harness and production pin freeze:
blocked. The workbook maps are engineering drafts only.

Still required: module VIN and logic levels; exact pin legends; connector identity,
pitch, mating and orientation; polarity and return paths; isolation; shared-ground
suitability; current budget/inrush; USB data capability; switch continuity; and
reviewed protection/fuse/eFuse strategy.

Do not connect active peripherals to ESP32 power/GPIO, energize `HW-010`, create a
common-ground or final harness, or connect battery, motion, STM32 or CAN paths.

## Blocked domains

- Power/first power: `BLOCKED_UNVERIFIED_POWER_HARDWARE`; unauthorized.
- ESP32: `WAITING_FOR_EVIDENCE`; spare-board identity incomplete and primary camera
  board not received.
- Camera: `BLOCKED_UNTIL_ARRIVAL_AND_REVIEW`.
- Wiring: `PROVISIONAL_ENGINEERING_PLAN_ONLY`.
- STM32/motion: `BLOCKED_MISSING_REQUIRED_HARDWARE_AND_EVIDENCE`.
- Controller link/CAN: no transceiver, controller clock, topology, bitrate, traffic,
  timing or physical-link evidence. Phase2B2 remains `WAITING_FOR_VERIFIED_INPUTS`.
- Physical commissioning: zero gates passed; autonomous motion remains blocked;
  robot authority remains `NONE`.
