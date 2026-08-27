# Modular Hardware Architecture

## Status and intent

This document defines the firmware dependency architecture introduced for ZI-E. It
does not freeze wiring or component-level bindings. The approved controller split is
preserved: ESP32-S3 owns multimedia/presence; STM32 owns deterministic safety/motion.

## Dependency rule

```text
AI / laptop behavior
  -> SafeRobotCommands (named intent, normalized request, lease)
  -> services (presence, motion orchestration)
  -> stable HAL interfaces
  <- concrete ESP32-S3 / STM32 drivers
  <- vendor SDK, buses, pins, registers
```

AI and reusable services cannot depend on GPIO, PWM, registers, vendor libraries,
concrete actuators, or board-specific classes. Hardware replacement therefore changes
a driver and profile binding, not unrelated robot logic.

## Stable capabilities

Required baseline interfaces are `Display`, `MotionController`, `Battery`,
`RangeSensor`, and `Audio`. Prepared optional interfaces are `Camera`,
`BellyLightMatrix`, `HeadMotion`, `Arms`, and `SafetySensors`
(cliff/collision).

Interfaces communicate semantic values and outcomes. They deliberately omit pin
numbers, bus addresses, vendor handles, raw duty cycles, register values, raw motor
power, and individual actuator setpoints.

## Ownership

| Capability | Owner | Safety role |
|---|---|---|
| Display, Audio, Camera, BellyLightMatrix | ESP32-S3 | Presence; matrix fails dark |
| Base/head/arm motion | STM32 | Safety/motion critical |
| Battery safety telemetry | STM32 | Safety critical |
| Cliff/collision range and contacts | STM32 | Local stop authority |
| Head measurement ranging | ESP32-S3 or delegated service | Assistance only; never replaces STM32 safety sensing |

The classical CAN/TWAI protocol between controllers is a future stable adapter
boundary. Protocol messages must carry high-level requests, leases, explicit execution
states, faults, and confirmed state -- not arbitrary actuator control.

## Registry and binding

`HardwareRegistry` is the dependency-binding object. Services receive interface
pointers and explicitly handle absent optional capabilities. Board composition roots
own concrete drivers and bind verified instances. Bindings carry controller-owner
metadata; profile validation rejects missing, unowned, or wrong-controller required
capabilities at startup.

- `bench-minimal`: baseline five capabilities, backed by deterministic host stubs.
- `stage1`: baseline plus STM32 safety sensing required; camera and belly matrix optional.
- `full-prototype`: baseline plus camera, belly matrix, head motion, arms, and safety
  sensors required.

No production profile silently substitutes a stub for missing hardware.

## Safe behavior boundary

`SafeRobotCommands` supports expression/audio cues, normalized base-motion requests
with finite-value and nonzero-lease validation, safe stop, privacy disable, and
protected-shutdown stow. Normalized intent is not raw actuator authority: STM32 remains
responsible for conversion to verified physical limits, arbitration, interlocks,
stale-command handling, feedback, and safe-stop behavior.
The maximum command lease and expiry timing are unresolved verified configuration
owned by STM32; the facade does not invent a duration limit. STM32 must reject leases
outside its configured policy and stop locally on expiry.

Protected-shutdown stow is one high-level `MotionController` request. STM32 owns its
sequencing, arbitration, interlocks, and actual-state confirmation; the service does
not synthesize completion from individual arm/head command acceptance.


Motion remains rejected until commissioning is confirmed. A driver must report
explicit execution state; command receipt is never completion.

Phase 2A adds a transport-independent lifecycle core ahead of the future controller
link. Commands carry nonzero source/session/sequence identity, one command owns a
motion subsystem at a time, duplicates do not renew leases, in-flight replacement is
rejected, and only defined accepted/armed/executing/terminal transitions are legal.
Every transition checks a caller-supplied monotonic time and an expired lease faults
locally. The lifecycle report does not replace the STM32 safe-stop actuator path or
actual-state confirmation.

Phase 2B1 adds a separate controller-link session guard. It binds liveness to the
configured peer controller, compatible protocol major version, peer boot session, and
ordered heartbeat sequence. Hello creates only a candidate session; the first valid
bound heartbeat grants motion authority. Duplicate/stale frames do not extend liveness.
Timeout or expected-peer restart removes authority, and renegotiation requires a fresh
heartbeat. Stray or incompatible Hellos cannot replace or fault a healthy session.
This is not yet a wire protocol and does not provide authenticated replay protection or
select CAN identifiers, payload layout, integrity mechanism, or timing values.

String identifiers are semantic catalog keys, not vendor commands. Drivers must
validate supported identifiers synchronously and must copy any value retained after
the call; future frozen catalogs may replace them with typed enums.

## Extension registry boundary

The host-side in-memory registry binds every accepted manifest and device identity to
an external package/trust/controller/profile assignment. It owns extension lifecycle
and capability exposure, while hardware profiles continue to own actual driver
composition. A capability resolves to an active/degraded registry record plus logical
device instance; ambiguous live providers are rejected rather than selected by load
order. Inactivity, failure, quarantine, disable, and removal clear exposure
synchronously.

Registry capability resolution grants no direct hardware handle. AI and plugins still
cross only semantic service/Safe Robot API boundaries; no raw GPIO, PWM, register,
actuator, vendor-driver, or safety-bypass permission exists. Loading, sandboxing,
runtime permission enforcement, persistence, and commissioning binding remain future
layers.

Transactional configuration is a separate in-memory snapshot boundary. It binds a
candidate to the registry's package, logical device, and profile identity, then stages,
validates, and atomically replaces only the configuration snapshot. Semantic hardware
settings do not receive driver handles and are not applied by this layer. Failed or
blocked updates preserve the previous snapshot; lifecycle and capability authority
remain with the registry.

The semantic Robot API sits above registry authorization and below a future core
dispatcher. It separates command intent, event occurrence, and current state. Plugin
commands carry stable source/session/sequence identity and require a fixed active
capability; events cannot dispatch work; consumers can only read generation-ordered
state snapshots. An accepted intent is not execution or actual-state confirmation, and
no semantic type exposes a driver or protected safety operation.

## Adding or replacing hardware

1. Confirm the capability and controller owner.
2. Implement the existing interface in the appropriate driver directory.
3. Keep vendor types and electrical bindings private to that driver.
4. Bind it in a board profile without fallback to guessed configuration.
5. Add contract tests with a stub/fake, then driver bench tests.
6. For motion/safety hardware, complete low-power commissioning and fault injection
   before normal autonomous use.

## Unresolved bindings

Production drivers remain TODO pending verified schematics, datasheets, and bench
results: all pins/buses; exact display FPC/touch controller; sensor addresses/topology;
battery telemetry/protection; CAN schema; actuator IDs, models, baud, voltage classes,
directions, zeros, ranges, current/thermal limits, end sensors and feedback
plausibility; audio routing; camera wiring; matrix current cap; safety timing thresholds.
