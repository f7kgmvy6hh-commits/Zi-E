# ZI-E Roadmap

## Phase A — Concept Design
Status: substantially complete.

- visual personality;
- operating poses;
- modularity direction;
- basic system architecture;
- service philosophy.

## Phase B — Design Freeze v0.1
Next.

- overall mechanical envelope;
- proportions → real dimensions;
- head motion/retraction envelope;
- arm motion/stow envelope;
- wheel/tuck envelope;
- battery/service envelope;
- internal zones;
- hard interfaces.

## Phase C — Component Verification
Review one component at a time.

For each:
- role;
- real datasheet;
- dimensions;
- voltage/current;
- interfaces;
- thermal/mechanical constraints;
- availability/cost;
- alternatives;
- KEEP / REPLACE / UNDECIDED.

## Phase D — Electrical Architecture
Only after component direction is sufficiently stable.

- power tree;
- charging;
- battery protection;
- current budget;
- buses/interfaces;
- controller resource budget;
- pinout;
- connectors;
- safety.

## Phase E — Digital Prototype / Simulation
Before full purchase.

- CAD fit;
- kinematic checks;
- arm/retraction collision checks;
- center of gravity estimates;
- electrical simulation where useful;
- firmware mocks;
- laptop brain protocol;
- virtual command/state testing.

## Phase F — Breadboard / Bench Bring-up

- power;
- controller;
- communications;
- one actuator at a time;
- sensors;
- audio;
- display/camera;
- integrated safety.

## Phase G — Mechanical Prototype

- print/build;
- assemble modules;
- validate motion;
- iterate clearances;
- verify serviceability.

## Phase H — AI Brain Integration

- wireless protocol;
- speech;
- vision;
- memory/database;
- behavior;
- safe intent layer.

## Phase I — Used Phone Migration

After laptop-based behavior/database is stable:
- edge compute feasibility;
- local database sync;
- phone↔robot protocol;
- offline behavior;
- cloud fallback;
- power/thermal strategy.


## Immediate pre-purchase risk-retirement step
Run the minimum-parts bench campaign in `PRE_PURCHASE_GATE.md` before ordering batches. Prioritize actuator commissioning/thermal tests, cliff surface tests, head cable loop, wheel traction/braking, battery transients, magnet pull/thermal, and belly optical coupons.

## 2026-08-28 handoff lanes

### SOFTWARE COMPLETE

Authority ownership, semantic APIs, extension/package lifecycle, device/profile
identity, modality-specific providers, presentation packs, runtime/recovery,
update/security policy, VirtualRobot integration, and protected safety separation are
remain frozen and repository-authoritative. The App command-center software foundation
now adds honest readiness/status APIs, a semantic HostRuntime adapter contract,
closed simulator/unavailable behavior, modality status views, and adversarial tests.

### SOFTWARE FUTURE WORK

Implement the privileged real-target adapter against HostRuntime/public semantic
services, including process/transport authentication, capability-scoped profile,
package/plugin, provider, configuration and pack operations, authoritative telemetry,
and confirmed command/E-stop feedback. Add no direct hardware path to the App.

### WAITING_FOR_VERIFIED_INPUTS

Phase 2B2 wire/CAN decisions, production drivers, pins/connectors, measured timing,
safe-stop performance, sensor behavior, actuator identity/load/thermal limits, battery
behavior, cable topology, and final received-part CAD interfaces remain
`WAITING_FOR_VERIFIED_INPUTS` / `VERIFY_ON_ARRIVAL`.

### VERIFY_ON_ARRIVAL

Received-part identity, dimensions, interfaces, pins, connectors, cable lengths,
measured loads/current/thermal behavior, sensor performance, battery behavior, and
CAD-dependent clearances remain arrival worksheets, not frozen facts.

### PHYSICAL COMMISSIONING REQUIRED

Logic-only bring-up, controller identity/session, sensors, one bounded actuator at a
time, direction/range/feedback, physical safe stop, constrained motion, and operator
evidence remain mandatory before supervised or autonomous physical motion.

### Current App foundation

The loopback command-center/HUD prototype now exists and is validated only for its
simulator/operator surface. Its eventual real target must bind only to frozen
HostRuntime/public semantic services for readiness, explicit profile selection,
package/plugin management, provider priorities, packs, semantic commands,
configuration, logs/events/diagnostics, commissioning, and VirtualRobot/real-target
selection. The public status/contract portion is complete; the privileged real-target
implementation remains future work. Until it exists, App state is not physical
authority or execution evidence.
