# Prototype Phase 1 Build-Ready Package — 2026-08-28

Status: process and worksheets are ready; execution is blocked on received hardware
and operator-confirmed measurements. Every physical value not backed by received-part
evidence is `VERIFY_ON_ARRIVAL`.

Canonical companion artifacts for this package:

- `HARDWARE_SOURCE_OF_TRUTH_RECONCILIATION.md`
- `PURCHASED_PARTS_INTAKE_SCHEMA.md` and its empty CSV template
- `P1_CAD_FREEZE_CANDIDATE.md`
- `P1_ASSEMBLY_ELECTRICAL_BRINGUP_PACKAGE.md`
- `P1_PHYSICAL_VERIFICATION_WORKSHEET.md`
- `PHASE2B2_INPUT_GATE.md`

These documents supersede generic rows below when more precise, without converting
CAD/listing assumptions into physical facts.

## P1 scope and controller map

P1 is a supervised bench prototype: power domains, ESP32-S3 multimedia/presence,
STM32 local safety/motion ownership, sensors, one actuator at a time, communication,
safe stop, and fault containment. It excludes autonomous motion and production claims.

| Owner | Responsibility | Forbidden boundary |
|---|---|---|
| ESP32-S3/host | multimedia, presence, providers, presentation, semantic intents, diagnostics | raw actuator or protected-safety control |
| STM32 | local interlocks, actuator gating, actual-state confirmation, safe stop | AI/provider authority |
| Operator | physical inspection, instrument readings, gate confirmation | replacing evidence with a software flag |
| VirtualRobot | software integration and fault simulation | claiming physical verification |

## BOM and arrival ledger

Use one row per exact purchasable part. Do not infer status from CAD presence.

| Item / exact model | Qty | Status: existing / required / ordered / substitute candidate | Evidence | Arrival inspection | Result |
|---|---:|---|---|---|---|
| ESP32-S3 multimedia controller | TBD | required | purchase record | `VERIFY_ON_ARRIVAL` exact module/revision/voltage/interfaces | pending |
| STM32 safety controller | TBD | required | purchase record | `VERIFY_ON_ARRIVAL` exact MCU/board/revision | pending |
| Display/camera/audio parts | TBD | required | component matrix | `VERIFY_ON_ARRIVAL` connector, dimensions, controller IC | pending |
| Sensors and actuator samples | TBD | required | component matrix | `VERIFY_ON_ARRIVAL` model, class, voltage, identity | pending |
| Power/BMS/protection parts | TBD | required | approved purchase decision required | `VERIFY_ON_ARRIVAL` ratings and provenance | pending |
| Fasteners/spacers/brackets | TBD | required | assembly drawing/cut list | `VERIFY_ON_ARRIVAL` thread, length, material, clearance | pending |

Substitutes require a documented interface, voltage, mechanical, thermal, safety, and
availability comparison plus Product Owner approval before use.

## Assembly order and mechanical checklist

1. Inventory and photograph labels; quarantine mismatches.
2. Measure parts before modifying CAD or brackets.
3. Assemble non-powered structure, service access, hard stops, guards, and strain
   relief before electronics.
4. Install protected power distribution with loads disconnected.
5. Install controllers and logic-only interfaces.
6. Bring up sensors, then individual actuator branches, then constrained integration.

Check fastener thread/length, washers, locking method, captive access, spacer height,
bracket flatness, sharp edges, insulation from conductors, bearing support, hard-stop
load path, tool clearance, and service-removal access. All dimensions are
`VERIFY_ON_ARRIVAL`.

## Power and connector architecture

High level only: source -> fuse/current limit -> protected distribution -> separately
switchable safety/motion, logic, and non-safety multimedia branches. STM32/local
safety must retain the means to remove actuator authority independently. The belly
matrix and cosmetic loads fail dark and cannot share a safety-critical single point of
failure. Final wire gauge, fuse, shunt, FET, connector, transient, and thermal values
are `VERIFY_ON_ARRIVAL` after measured load waveforms.

| Interface | End A / owner | End B / owner | Semantic purpose | Physical details |
|---|---|---|---|---|
| ESP32–STM32 | ESP32 host | STM32 safety | bounded semantic command/state/liveness | CAN/TWAI details `VERIFY_ON_ARRIVAL`; Phase 2B2 waiting |
| Display/camera/audio | ESP32 | multimedia devices | presentation/perception | pins/connectors `VERIFY_ON_ARRIVAL` |
| Safety sensors | STM32 | cliff/bumper/power sensors | local interlock evidence | pins/levels `VERIFY_ON_ARRIVAL` |
| Actuator branches | STM32-authorized interface | one bounded actuator/domain | commissioned motion only | bus/IDs/baud/voltage `VERIFY_ON_ARRIVAL` |

Preliminary pin worksheet fields: signal, semantic owner, board pin, direction,
voltage, pull state, boot behavior, connector/cavity, protection, cable length,
source drawing, measured evidence, reviewer. Every populated physical pin remains
`VERIFY_ON_ARRIVAL` until continuity and board documentation agree.

## Flashing checklists

ESP32-S3: verify exact board and USB mode; obtain authoritative board instructions;
record toolchain/firmware digest; disconnect actuator power; flash a logic-only image;
capture boot log; verify reset/recovery path. Port, boot pins, and image target are
`VERIFY_ON_ARRIVAL`.

STM32: verify exact MCU/debug connector/voltage; connect current-limited logic power;
preserve recovery/debug access; flash a non-motion commissioning image; record
toolchain/image digest and option-byte state; verify reset/watchdog behavior. Probe
pinout and board settings are `VERIFY_ON_ARRIVAL`.

## Bench bring-up and first power

1. Unpowered continuity, polarity, isolation, fastener, cable, and foreign-object
   inspection; actuator branches disconnected.
2. Current-limited source at the verified logic requirement; operator at disconnect;
   record idle/inrush voltage/current/temperature.
3. Verify controller identity, logs, watchdog, and safe outputs.
4. Verify each sensor independently, including plausible change and disconnected/stuck
   faults.
5. Keep actuator power disabled while discovering exact model/voltage/bus/baud/ID.
6. Enable one restrained actuator at low configured limits; verify direction, zero,
   range, feedback, stop, and power removal before the next actuator.
7. Test constrained multi-subsystem behavior only after every prior gate passes.

Smoke tests: controller boot/recovery, state/event generation, sensor plausible/fault
states, display/audio fail-safe degradation, provider mocks, command accepted versus
executed distinction, watchdog, safe-stop input, and loss-of-link behavior.

## Commissioning state machine

`unpowered -> inspection_complete -> logic_power_only -> controller_connected ->
sensors_verified -> actuator_power_disabled -> individual_actuator_test ->
constrained_motion_test -> safety_stop_verified -> bench_integrated ->
ready_for_supervised_prototype`

Each transition requires recorded operator/physical evidence plus the relevant test;
simulation success is a separate field. A failed gate removes actuator authority and
returns to `actuator_power_disabled` or `unpowered` as the failure requires. Individual
tests have explicit time/current/range bounds derived from verified hardware. No
software flag can assert a physical transition by itself. Autonomous motion remains
prohibited throughout P1.

## Test and evidence worksheets

- Sensors: identity, supply, raw/plausible response, limit cases, stale/disconnect,
  environmental matrix, pass/fail, operator/time.
- Actuators: model/voltage, bus/baud/ID, direction, zero/range, end sensors, plausible
  feedback, current/temperature, bounded duration, stop response.
- Safe stop: trigger type, starting condition, command rejection, physical power/state,
  measured stop timing (`VERIFY_ON_ARRIVAL`), restart authority remains absent.
- Measurements on arrival: exact revision, dimensions, mass/COM where relevant,
  connector/pinout, voltage/current, thermal observation, photo/datasheet provenance.
- Cable/strain relief: bend radius, moving loop, edge protection, service loop,
  connector retention, flex cycles, separation from noisy power.
- Fault containment: unplug/open/short where safely instrumented, stuck sensor, bus
  loss, controller reset, optional-device failure, branch fuse/current limit, thermal
  and e-stop/disconnect response.

## CAD readiness

The current parameterized v0.3 CAD and STEP/STL exports support fit coupons, envelope
review, and bench fixtures. The architecture, state variants, service zones, protected
controller split, and parametric workflow are usable. Physical clearances, display and
sensor geometry, head/arm mass and COM, actuator ratios, cable loops, battery/power
hardware, final shell surfaces, and received-part-dependent dimensions remain
`VERIFY_ON_ARRIVAL`. Existing mass, torque, stability, and thermal figures are model
outputs/estimates, not measurements. No geometry is fabricated by this stage.

## Go/no-go gates

No supervised constrained motion until identity, voltage class, bus/baud, direction,
zero/range, end-state sensors, plausible feedback, current limiting, restraint, and
manual stop pass. No integrated bench state until every required sensor and local
safe-stop path passes. No autonomous motion until a later separately approved hardware
commissioning phase validates physical timing and all safety gates. VirtualRobot,
received-command, or software-ready status is never sufficient.
