# Phase 2 Execution Plan

## Objective

Turn the Phase 1 interface scaffold into a testable safety/control foundation without
freezing unverified electronics or exposing raw actuator authority.

## Phase 2A — Safety contract foundation

Inputs: Phase 1 interfaces, commissioning rule, execution-state requirement, and
open-source failure harvest.

Expected output:

- transport-independent command/source/session identity;
- single-owner motion lease arbitration;
- explicit legal execution-state transitions;
- duplicate/idempotency, stale/out-of-order rejection, and explicit refusal to
  replace an in-flight command;
- deterministic local lease-expiry detection;
- host contract tests and documented invariants.

Allowed files: `firmware/include/zie/core/`, `firmware/src/`, `firmware/tests/`,
`firmware/CMakeLists.txt`, and Phase 2/project-state documentation.

Constraints: no vendor SDK, wire format, GPIO, bus address, raw PWM, individual joint
setpoint, guessed timeout, or production fallback. Time is supplied by the caller so
the core remains deterministic and testable.

Validation: compile with C++17; run host tests; cover invalid identity/lease, duplicate
request, competing owner, stale sequence, illegal transition, terminal release, and
lease expiry on both polling and transition paths. If the toolchain is unavailable,
record that explicitly and do not call the tests passed.

## Phase 2B — Controller-link protocol

Inputs: approved 2A invariants, verified controller/bus direction, and measured timing
needs.

Expected output: versioned semantic messages for command, execution report, heartbeat,
fault, and confirmed state; boot/session negotiation; bounded serialization; integrity
checks appropriate to the selected transport; compatibility manifest; golden vectors,
parser fuzzing, reboot/replay tests, link-loss tests, and bus fault injection.

Constraints: transport adapters must not weaken 2A semantics. No arbitrary actuator
frames. Heartbeat loss and peer reboot must lead to a deterministic local safe state.
Cross-session replay rejection is a 2B responsibility: 2A distinguishes session IDs
but does not decide which newly negotiated session is authoritative.

Freeze gate: select CAN/TWAI framing and timeout values only after bus-load analysis
and bench measurements.

## Phase 2C — Production binding and commissioning

Inputs: verified schematics/datasheets, 2B protocol, actuator identity/range data, and
bench measurements.

Expected output: STM32 safety-motion composition root, ESP32 presence composition root,
low-power commissioning workflow, commissioning record tied to hardware/configuration
identity, diagnostic bundle, and hardware-in-loop/fault-injection results.

Constraints: normal motion remains locked until identity, voltage class, bus/baud,
direction, zero/range, end-state sensors, and feedback plausibility pass. Received or
accepted never means executed.

Freeze gate: no autonomous motion or batch purchase based only on host simulation.

## First coherent foundation

Implement only the Phase 2A command lifecycle core and its host tests. Integration
into `MotionService`, wire encoding, heartbeat transport, production profiles, and
hardware drivers remain deliberately out of scope for this commit.
