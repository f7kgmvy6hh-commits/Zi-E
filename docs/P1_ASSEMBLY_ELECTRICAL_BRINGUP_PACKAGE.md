# P1 Assembly, Electrical Preparation, and First-Power Package

Every blank value, physical assignment and rating is `VERIFY_ON_ARRIVAL` unless linked
to received-part evidence. Autonomous free-driving is prohibited.

## Mechanical/module assembly order

1. Reconcile inventory IDs, labels, photos and datasheets; quarantine mismatches.
2. Measure parts and print fit/interface coupons before editing production geometry.
3. Dry-assemble lower chassis, torso, service covers, feet and passive supports.
4. Verify sharp-edge removal, insulation barriers, hard-stop load paths and access.
5. Prepare controller/power standoffs and removable trays without powered parts.
6. Prepare wiring corridors, grommets, clamps, moving loops and service loops.
7. Install protected power distribution with every downstream load disconnected.
8. Install STM32 safety controller, debug access and logic-only harness.
9. Install ESP32 multimedia controller, antenna/USB access and logic-only harness.
10. Install one sensor branch at a time; defer actuators, display/audio/camera loads.
11. Install one mechanically restrained actuator branch at a time after identity tests.
12. Install display/camera/audio and optional cosmetic loads after required safety
    paths are verified. Close covers only after inspection and thermal checks.

Service removal order is the reverse, with source disconnected first. Battery,
power-distribution, debug, safe-stop and controller access must not require removing a
loaded moving mechanism.

## Fastener/spacer/bracket matrix

| Location/module | Fastener/thread | Length/stack | Spacer/insert/bracket | Retention/torque | Evidence/status |
|---|---|---|---|---|---|
| Chassis/torso shells | TBD | TBD | TBD | TBD | `VERIFY_ON_ARRIVAL`; print coupon/load path |
| Rear service cover | TBD | TBD | captive/serviceable TBD | TBD | repeated-removal test |
| Battery cassette/door | TBD | TBD | insulated restraint TBD | TBD | impact/retention/service test |
| ESP32/STM32/power trays | TBD | TBD | insulated standoffs TBD | TBD | exact hole pattern/clearance |
| Head rails/carriage/pan/tilt | TBD | TBD | bearing/yoke/hard-stop TBD | TBD | alignment, obstruction, cycle test |
| Shoulder/elbow/arm shells | TBD | TBD | yokes/bearings/belt brackets TBD | TBD | load/thermal/backlash test |
| Wheel pods/caster/feet | TBD | TBD | lock/hard-stop brackets TBD | TBD | traction/braking/fold-load test |
| Sensor/optical windows | TBD | TBD | light seals/carriers TBD | TBD | FOV/crosstalk/retention test |
| Cable clamps/strain relief | TBD | TBD | grommet/clamp TBD | TBD | bend/flex/pull test |

## Removable modules and fault containment

- Removable: battery cassette, rear electronics tray/cover, controller trays, wheel
  pods, arm modules, head rear hatch/sensor carrier, gripper/tools and optical insert.
- Each module receives identity labels and keyed/polarized connectors where the final
  verified connector system supports them.
- Separate logic, protected safety/motion, multimedia/cosmetic and high-current
  actuator branches. A cosmetic/optional fault must not remove local safety.
- Protect harnesses from pinch/shear, moving joints, sharp edges, heat and speaker/
  motor interference. Provide service slack without uncontrolled loops.
- Branch protection, disconnect and current-limit values remain TBD pending measured
  loads. Fault injection occurs only with appropriate current limitation and isolation.

## Power-tree worksheet

```text
received battery/bench source
  -> primary disconnect/protection [type/value TBD]
  -> protected distribution
       -> STM32 safety logic rail [regulator/protection TBD]
       -> actuator/drive branches [individually gated/protected; values TBD]
       -> ESP32/multimedia logic rail [regulator/protection TBD]
       -> display/camera/audio/belly optional branches [individually bounded]
```

| Node/branch | Source/load | Nominal/min/max V | idle/inrush/peak A | protection/fuse | wire/connector | measured evidence |
|---|---|---|---|---|---|---|
| Bench source / pack input | TBD | TBD | TBD | TBD | TBD | blank until measured |
| STM32 safety logic | TBD | TBD | TBD | TBD | TBD | blank until measured |
| ESP32/multimedia logic | TBD | TBD | TBD | TBD | TBD | blank until measured |
| Left/right drive | TBD | TBD | TBD | TBD | TBD | blank until measured |
| Head/arm/tool branches | TBD | TBD | TBD | TBD | TBD | blank until measured |
| Display/camera/audio/belly | TBD | TBD | TBD | TBD | TBD | blank until measured |

Common-reference/ground strategy must be schematic-reviewed. Record return paths,
shield/drain treatment, chassis connection if any, logic/power separation, ground-loop
risks and measurement points. Never infer common-ground suitability from connector
fit alone.

## Interface, connector and wiring worksheet

| Link | Semantic owner | Interface class documented | Final pins/connector | Cable/topology/separation | Gate |
|---|---|---|---|---|---|
| ESP32–STM32 | both; STM32 safety authoritative | Classical CAN/TWAI direction | TBD | topology/termination/length TBD | Phase 2B2 waiting |
| STM32–cliff/bumper/power | STM32 | sensor/discrete classes | TBD | separate/protect from motor noise | verify one at a time |
| STM32–drive/arms/head/tools | STM32 | protected actuator/tool classes | TBD | high-current separation and branch disconnect | commission individually |
| ESP32–display/camera/audio/belly/head range | ESP32 | multimedia/sensor classes | TBD | RF/audio/optical and moving-loop constraints | verify exact modules |
| Debug/flash/service | respective controller | USB/debug class TBD | TBD | accessible with actuator power disabled | verify board docs |

Final pins, connectors, gauges, fuse/current-limit/regulator values, CAN parameters and
measured deadlines are not frozen by this worksheet.

## First-power and bench sequence

| Gate | Required action/evidence | Failure action |
|---|---|---|
| 1 Visual/mechanical | labels/photos, polarity marks, foreign objects, insulation, fasteners, restraints, clearances | remain unpowered; quarantine/rework |
| 2 Continuity/short | source rails to return, rail isolation, polarity, chassis/ground plan, branch disconnects | remain unpowered; locate fault |
| 3 Isolated source | current-limited bench source, verified range, operator disconnect, no battery/actuator loads | disconnect immediately on anomaly |
| 4 Rails/regulators | verify each unloaded rail then representative bounded load; record V/I/temperature | disable branch; redesign sizing/protection |
| 5 Controller-only | logic-only STM32 carrier identity/debug/reset/watchdog/safe outputs | remove power; no actuator connection |
| 6 ESP32 boot | exact board/USB/boot/recovery/logs with optional loads absent | remove optional load; preserve STM32 safety |
| 7 STM32 boot | commissioning firmware, debug, reset/watchdog and actuator gates disabled | unpowered recovery; no motion |
| 8 Communication | semantic session/liveness only; loss/reboot cannot arm motion | remain actuator-power-disabled |
| 9 Sensors | one sensor at a time: identity, plausible/stale/disconnect/stuck response | mark failed; no integrated gate |
| 10 Actuators | one restrained actuator: identity/model/voltage/bus/baud/ID/direction/zero/range/feedback/current/temperature | cut branch power; return to disabled |
| 11 Multimedia | display/audio/camera/range/belly individually, optional failure isolated | degrade optional branch only |
| 12 Safe fault injection | disconnect/reset/stuck/open and protected shorts only where instrumented | physical disconnect; inspect |
| 13 Safe-stop observation | trigger, command rejection, actual state/power feedback, measured timing blank until observed | no further motion; diagnose |
| 14 Integrated bench | required sensors + safety + bounded semantic commands while restrained | return to actuator-power-disabled |
| 15 Restrained motion | only after signed prior gates; reduced verified limits, one behavior at a time | immediate safe stop and rollback |

No autonomous free-driving occurs in P1 first bring-up. A received/accepted command is
not executed until actual state confirms it.

