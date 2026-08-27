# Semantic Robot API and Command/Event/State Foundation

Status: in-memory C++17 contracts only. Accepted commands are validated authorization
intents; this stage has no dispatcher, scheduler, IPC, transport, networking, CAN,
driver, persistence, UI, or AI provider.

## Command

`SemanticCommand` is a typed intent envelope containing package/logical-device source,
nonzero session and sequence identity, command type, and matching variant payload. A
numeric session carried by a command is not authority. `AuthoritativeRobotCore` must
explicitly bind the active session to the exact registry package and logical device
before submission. Replacement retires the prior session, retired IDs cannot be
rebound, and sequence ordering restarts only for a newly core-bound session. The fixed
command catalog covers normalized motion, semantic stop, expression/presentation,
audio/speech, and sensor query. Command type selects its required capability internally;
the caller cannot substitute a weaker capability string.

Acceptance requires an exact registry package/device match, lifecycle `active`, the
mapped active capability, a valid typed payload, and a newer sequence for that source
session. Normalized motion is finite and bounded to `[-1, 1]` with a nonzero lease.
Protected safety operations are explicitly rejected for plugin callers. Unknown types
fail closed. There is no raw motor setpoint, PWM, GPIO, register, actuator ownership,
raw frame, or safety-bypass command.

## Event

`RobotEvent` is a copied immutable occurrence record with nonzero event ID, typed
category, typed source class, source ID, and detail. Publishing appends to an event
journal only. It has no reference to command dispatch or state mutation, so an event
cannot cause an action merely by being published. Unknown categories/sources fail
closed.

## State

`RobotStateSnapshot` is a semantic read model keyed by typed category and monotonically
increasing generation. Consumers receive only `const` snapshots from `RobotStateStore`.
Writes go through the separate authoritative core writer, never an extension-supplied
authority claim. Zero, stale, duplicate, malformed, and unknown-category snapshots are
rejected without changing current state.

## Integration boundary

This layer is intended to authorize semantic intents before a later authoritative core
dispatcher invokes existing Safe Robot services. Acceptance is not execution or
completion. STM32 remains owner of motion/safety interlocks, physical limits,
commissioning, actual-state confirmation, and protected operations.

Accepted commands cross into a later core adapter only as an opaque
`AcceptedSemanticCommand` token constructed by `SemanticRobotApi`; callers cannot
manufacture that token or bypass authorization by invoking the virtual adapter. The
`AuthoritativeRobotCore` definition is kept outside the public `include/zie` API tree;
it is an internal composition authority and must not be included in a plugin SDK.
