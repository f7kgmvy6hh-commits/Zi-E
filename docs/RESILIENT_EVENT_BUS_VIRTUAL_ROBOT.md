# Resilient Event Bus and Virtual Robot Foundation

Status: deterministic synchronous C++17 host contracts only. There are no threads,
transport, scheduler, persistence, hardware drivers, autonomous behaviors, or physical
safety claims.

## Event bus contract

Subscriptions have deterministic subscriber, package, and logical-device identity; a
closed event-category filter; and an optional required active capability. Registration
rejects duplicate subscriber IDs, unknown categories/policies, registry impersonation,
inactive owners, and missing capabilities. Eligibility is checked again at publish and
delivery, so quarantine, disable, removal, or another inactive lifecycle clears queued
extension delivery rather than leaking old events.

Each subscriber has a fixed-depth queue. The sole current policy is deterministic
`drop_newest`: the queued event is preserved, the incoming event is dropped for that
subscriber, and `published_with_overflow` is returned. There is no unbounded queue.
Delivery copies immutable `RobotEvent` data and catches callback exceptions, returning
`subscriber_failed` without corrupting other queues or subscribers. Publishing has no
reference to command submission or `RobotStateStore`, so it cannot execute a command or
authoritatively mutate state.

## Virtual robot contract

`VirtualRobot` consumes only opaque accepted-command tokens from `SemanticRobotApi`.
Command submission and simulated execution are separate steps. Successful semantic
motion, stop, presentation, audio, and sensor-query operations update generation-ordered
state through `AuthoritativeRobotCore` and publish immutable result events. Per-operation
success/failure is deterministic and configurable in memory; simulated failure emits a
rejection event without changing the associated state.

The authoritative session/state writer is an internal core header outside the public
`include/zie` contract and is not part of a future plugin SDK surface.

The adapter exposes no GPIO, PWM, register, CAN, raw motor setpoint, actuator ownership,
or protected safety operation. It does not weaken registry authorization and does not
simulate STM32 safety ownership, physical dynamics, actual-state confirmation, or
autonomous motion.
