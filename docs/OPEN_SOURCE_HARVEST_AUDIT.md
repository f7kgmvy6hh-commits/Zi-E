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

## Phase 2 protocol/control harvest — 2026-08-27

| Problem | Comparable project / failure evidence | Adapted ZI-E mitigation | Verification before freeze |
|---|---|---|---|
| External setpoints can look healthy while a heartbeat is missing or jittering | PX4 Offboard requires continuous proof-of-life; PX4 issue #16944 reports intermittent offboard-loss failsafe despite high apparent publish rates | STM32 owns a monotonic lease expiry independent of command receipt; accepted state never suppresses link-loss handling | Deterministic expiry tests in 2A; jitter, dropped-frame, and delayed-frame tests in 2B |
| Middleware reconnect is not automatic and reconnection paths can churn or hang | micro-ROS Arduino issues #572, #912, #1502, and #1759 | Safety behavior cannot depend on micro-ROS/agent availability; peer reboot/session and reconnection are explicit 2B states | Kill/restart peer, cable interruption, partial entity creation, and repeated reconnect tests |
| Active controllers on inactive/failed hardware can cause unexpected motion | ros2_control Controller Manager defaults reject activation on inactive hardware and deactivate controllers on hardware error | Keep commissioning/health gating local and make illegal lifecycle transitions fail closed | Transition-table tests in 2A; hardware-error and restart tests in 2C |
| Duplicate suppression becomes unsafe around sender state loss or reboot | OpenCyphal/CAN uses transfer IDs and documents transfer-ID timeout/restart edge cases | Separate source, boot session, and sequence identity; do not infer a reboot from sequence wrap | Duplicate/out-of-order tests in 2A; boot/session negotiation and replay tests in 2B |
| One healthy loop can mask a stalled safety task | Zephyr task watchdog supports per-task channels with optional hardware-watchdog fallback | Plan independent supervision of safety/control tasks; transport heartbeat is not a substitute for task watchdogs | Stalled-task fault injection in 2C |

Phase 2 references:

- PX4 Offboard Mode: https://docs.px4.io/main/en/flight_modes/offboard.html
- PX4 issue #16944: https://github.com/PX4/PX4-Autopilot/issues/16944
- ros2_control Controller Manager: https://control.ros.org/master/doc/ros2_control/controller_manager/doc/userdoc.html
- micro-ROS Arduino issue #572: https://github.com/micro-ROS/micro_ros_arduino/issues/572
- micro-ROS Arduino issue #912: https://github.com/micro-ROS/micro_ros_arduino/issues/912
- micro-ROS Arduino issue #1502: https://github.com/micro-ROS/micro_ros_arduino/issues/1502
- micro-ROS Arduino issue #1759: https://github.com/micro-ROS/micro_ros_arduino/issues/1759
- OpenCyphal specification: https://specification.opencyphal.org/Cyphal_Specification.pdf
- libcanard transfer-ID behavior: https://github.com/OpenCyphal/libcanard/blob/master/libcanard/canard.h
- Zephyr task watchdog: https://docs.zephyrproject.org/latest/services/task_wdt/index.html
