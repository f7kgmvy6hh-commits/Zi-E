# App / HostRuntime Adapter Contract

Status: software contract implemented with simulation, unavailable-real, and
deterministic fake adapters. No physical adapter or transport is implemented.

## Boundary

The App consumes only `HostRuntimeAdapter` semantic methods: copied runtime status,
semantic command submission, semantic E-stop request, and bounded polling. The
contract exposes no ESP32, STM32, CAN, GPIO, PWM, register, driver, actuator, raw
setpoint, authority mutation, profile rebinding, or safety-bypass method.

The App cannot mint an authority generation or session. A future real adapter must
obtain both from HostRuntime and report current readiness on every operation. The App
compares target mode, authority generation, and session before and after a request;
any change fails closed before an event can claim execution. Configuration alone can
select simulation or unavailable-real mode but cannot install real authority.

## Closed target and result model

Targets are the closed semantic `RobotState` domain. Requests contain only semantic
target, bounded timeout, and source. Responses distinguish `requested`, `accepted`,
`rejected`, and `confirmed`, and independently report delivery and physical
confirmation. Physical confirmation may be true only when copied from authoritative
runtime feedback. Simulation is always labeled simulation and never physically
confirmed.

E-stop follows the same rule. Without a real adapter it is an operator request that
returns `requested_not_delivered`; delivery and physical confirmation are false.

## Public status

- `simulation`: deterministic local simulation, no physical claim.
- `real-target-unavailable`: `DISCONNECTED`, no session/generation, no delivery.
- `future-real-target`: reserved for a commissioned adapter reporting a live
  HostRuntime session, positive authority generation, availability, and readiness.

Profile, configuration, package/plugin, provider, and presentation operations remain
capability-scoped HostRuntime services. This stage exposes honest read-only
`not_exposed`/`unavailable` views; it does not create mutation endpoints or silently
bind a profile/device/pack/provider.

Conversational adapters currently executed inside the App are labeled
`app-local-non-robot` and `authorization: not_verified`. They cannot grant or mask
robot authority. Their local fallback behavior does not mutate the frozen HostRuntime
provider chain.

## Deferred

The privileged implementation, process/transport isolation, authentication between
App and runtime, persistence, real telemetry, hardware drivers, CAN/framing, physical
E-stop confirmation, and commissioning remain future work. Phase 2B2 remains
`WAITING_FOR_VERIFIED_INPUTS`.

Future authenticated real transport is responsible for semantic host requests plus
session, authority-generation and profile/configuration identity; copied ESP32
readiness, events, delivery and controller-link state return to the Host. ESP32–STM32
messages remain unavailable until Phase2B2 freezes a bounded verified contract. No raw
socket, GPIO, PWM, motor or CAN frame is exposed through the App/HUD adapter.
