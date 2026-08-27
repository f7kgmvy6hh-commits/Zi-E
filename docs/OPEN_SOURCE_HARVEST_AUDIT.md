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

### Phase 2B1 delta — 2026-08-27

| Project/source | Concept and real failure | Adapted ZI-E idea | Fit, provenance, security, decision |
|---|---|---|---|
| micro-ROS Arduino issue #1809 | A client reconnects initially but fails on later attempts, showing that reconnect state cannot be assumed clean | Treat peer restart as an explicit fault; remove motion authority and require local renegotiation instead of silently replacing the active session | Failure evidence only; no code copied. micro-ROS's dynamic entity model is not adopted. Prevents stale authority but needs denial-of-service testing. **ADAPT** |
| OpenCyphal Specification v1.0 | Transfer ordering uses bounded identifiers and anonymous sources cannot provide enforceable uniqueness/ordering | Require configured nonzero controller identity, boot-session identity, and serial-number arithmetic for heartbeat order | Specification concept only; no code copied. Zi-E is not adopting the full Cyphal stack. **ADAPT** |

Additional references:

- micro-ROS Arduino issue #1809: https://github.com/micro-ROS/micro_ros_arduino/issues/1809
- OpenCyphal Specification v1.0 (2025-05-16): https://specification.opencyphal.org/Cyphal_Specification.pdf

### Phase 2B2 bus-load/timing delta — 2026-08-27

| Project/source | Concept and failure | Adapted Zi-E idea | Fit, provenance, security, decision |
|---|---|---|---|
| Espressif ESP32-S3 TWAI documentation and FAQ | ESP32-S3 has Classical TWAI, not integrated CAN-FD; frames are limited to 8 data bytes. Driver alerts expose arbitration loss, RX queue full, errors, error-passive, and bus-off | Keep the approved Classical CAN boundary, but require bounded fragmentation, queue/error telemetry, and bus-off fault injection before wire freeze | Primary vendor documentation; no code copied. CAN fault confinement is useful but does not authenticate peers. **ADOPT facts / ADAPT handling** |
| Bosch CAN Specification 2.0 | Variable bit stuffing and non-preemptive arbitration make data-dependent frame time and priority interference unavoidable | Use conservative stuffed-frame bounds plus priority-aware response-time analysis, not average payload bandwidth | Protocol specification; calculation only. Does not supply Zi-E physical timing or security. **ADAPT** |
| STM32G0B1 datasheet and ST AN5348 | STM32G0B1 FDCAN can support the chosen Classical CAN peer, but controller capability does not prove board timing or physical-layer validity | Verify Classic-mode timing, clock, pins, transceiver, filters, and measured queue/ISR latency on the selected board | Primary vendor documentation; no code copied. **ADAPT** |

Decision: **DEFER** Phase 2B2 wire/framing implementation until the bus/timing freeze
criteria in `PHASE2B2_BUS_TIMING_ANALYSIS.md` are satisfied.

### Extension manifest foundation delta — 2026-08-27

| Project/source | Concept and failure | Adapted Zi-E idea | Fit, provenance, security, decision |
|---|---|---|---|
| Home Assistant config flows/unique IDs | Stable integration-scoped identity prevents duplicate setup; versioned config entries support migration rather than startup-time guesswork | Keep manifest/plugin identity distinct from future physical device and instance identity; defer device identity to its own foundation | Documentation concepts only; no code copied. Home Assistant's Python runtime model is not suitable for deterministic embedded modules. **ADAPT** |
| VS Code extension manifest and Workspace Trust | Executable entrypoints and data contributions have different risk; opening untrusted content can trigger unintended code execution unless trust gates activation | Separate executable host plugins from data-only packs; registry assigns trust and later runtime policy must enforce it | Documentation concepts only; no code copied. Desktop extension permissions are not robot safety authority. **ADAPT** |
| VS Code extension runtime security | Signatures, blocklists, publisher controls, and secret scanning complement manifests; declarations alone do not create a sandbox | Keep signature/revocation/provenance and secret scanning as later mandatory registry/package work; do not claim current permission requests are enforced | Documentation concepts only; no code copied. **INVESTIGATE** |
| ESPHome schema deprecation guidance | External components can break across schema/API evolution | Version Plugin API and manifest schema independently and reject unsupported required versions | Documentation concepts only; no code copied. ESPHome code generation is not adopted wholesale. **ADAPT** |

### Device identity foundation delta — 2026-08-27

Problem: device identity becomes unsafe when names or transport locations are treated as
permanent, when physical and logical identity collapse into one key, or when extensions
can claim protected ownership.

| Comparable project / failure report | Material finding | Adapted ZI-E mitigation | Verification / disposition |
|---|---|---|---|
| [Home Assistant device registry](https://developers.home-assistant.io/docs/device_registry_index/) and [collision implementation](https://github.com/home-assistant/core/blob/dev/homeassistant/helpers/device_registry.py) | External identifiers/connections and registry-generated device IDs are separate; identifier/connection collision errors are explicit | Separate physical identity from registry-generated logical instance and fail the whole candidate set on collision | Focused duplicate physical/logical host tests. Concepts only; no code copied. **ADAPT** |
| [Home Assistant entity-registry RFC and failure motivation](https://github.com/home-assistant/core/issues/11533) | Name-derived IDs change or swap when equal names load in different orders | Exclude display name and discovery order from stable identity | Type has no display-name field; discovery-order provenance is explicitly ephemeral and rejected. **ADAPT** |
| [ROS 2 composition design](https://github.com/ros2/design/blob/gh-pages/articles/150_roslaunch.md) | A loaded logical node instance receives an immutable unique ID and duplicate full names are rejected | Give logical device instances their own registry-generated ID and reject mutation after activation | Positive identity and post-activation mutation tests. Concepts only; no code copied. **ADAPT** |
| [systemd machine ID setup](https://www.freedesktop.org/software/systemd/man/250/systemd-machine-id-setup.html) | Transient identity is explicitly distinct from committed persistent identity | Model untrustworthy hardware serial absence as explicit provisional-local state, never as a transport-derived permanent ID | Positive provisional and negative source/provenance tests. **ADAPT** |

Decision: adopt the typed, in-memory validation boundary. Defer persistence and explicit
migration/rebind mechanics. This is host simulation only; bench verification remains
required before identity participates in commissioning or motion ownership. Package,
controller, profile, and trust assignments are authoritative external validation
context, not candidate claims.

### Extension registry/lifecycle delta — 2026-08-27

| Comparable project / real failure | Material finding | Adapted ZI-E mitigation | Verification / disposition |
|---|---|---|---|
| [Home Assistant entity platform](https://github.com/home-assistant/core/blob/dev/homeassistant/helpers/entity_platform.py) and [entity-registry RFC](https://github.com/home-assistant/core/issues/11533) | Duplicate stable IDs are ignored rather than assigned by load order; name/discovery order is not authoritative ownership | Reject duplicate package/physical/logical identity and retain removal tombstones; bind resolution to package plus logical device identity | Duplicate, impersonation, identity-conflict, and ambiguity host tests. Concepts only; no code copied. **ADAPT** |
| [ROS 2 lifecycle invalid-transition crash report](https://github.com/ros2/rclpy/issues/1209) | An externally requested invalid transition can throw and terminate a managed node instead of returning a contained failure | Reject unknown/unlisted transitions as result values without mutating registry state | Illegal and out-of-domain transition tests. Concepts only; no code copied. **ADAPT** |
| [ROS 2 lifecycle partial-transition failure report](https://github.com/ros2/rclcpp/issues/1880) | Mutating state before a transition operation succeeds can strand the system in an intermediate state | Validate the complete in-memory transition and failure-domain request before mutating lifecycle; activation commits state/capabilities only after subset and ambiguity checks | Failed activation/transition tests confirm prior record state and exposure remain unchanged. No callbacks or distributed transaction are claimed. **ADAPT** |
| [ROS 2 managed lifecycle demo](https://github.com/ros2/demos/blob/rolling/lifecycle/README.rst) | Managed functionality is exposed only while active; activation/deactivation callbacks prepare and tear down resources | Resolve only active/degraded capabilities and synchronously clear exposure on inactive/failure/quarantine/disable/removal | Positive host/embedded lifecycles and synchronous revocation tests. Concepts only; no code copied. **ADAPT** |

Decision: adopt the in-memory registry/lifecycle/capability boundary. Persistence,
loader callbacks, crash recovery, runtime permission enforcement, commissioning, and
hardware binding remain future verification stages.

References:

- Home Assistant config flow and unique ID guidance: https://developers.home-assistant.io/docs/core/integration/config_flow/
- VS Code extension manifest: https://code.visualstudio.com/api/references/extension-manifest
- VS Code Workspace Trust: https://code.visualstudio.com/api/extension-guides/workspace-trust
- VS Code extension runtime security: https://code.visualstudio.com/docs/configure/extensions/extension-runtime-security
- ESPHome schema deprecation guidance: https://developers.esphome.io/blog/2025/05/14/schema-deprecations/
