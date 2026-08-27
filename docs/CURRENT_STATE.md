# Current State — 2026-08-27

## Phase
Prototype architecture and Project/CAD v0.3 package are prepared. Phase 2A now has a
transport-independent motion command lifecycle foundation plus a Phase 2B1
controller-session/liveness scaffold. Bounded wire encoding, transport binding,
production bindings, and bench commissioning remain next. Physical risk-retirement
bench rigs are still required before bulk component purchase.

## What is substantially defined
- Overall structural architecture and service philosophy.
- Digital face / camera / dedicated head laser-rangefinder / audio architecture.
- Hidden secret-until-lit Belly Light Matrix (BLM) for RGB ambient light + low-resolution information such as clock/battery/status.
- ESP32-S3 multimedia and STM32 safety/motion split.
- Head Pan/Tilt/Lift mechanisms and operating states.
- Two-wheel differential base, folding wheel pods, passive caster, shutdown feet.
- Dual functional arm architecture with candidate actuators.
- Adaptive gripper and magnetic quick-swap tools.
- Battery placement/configuration, V1 charging approach and protected power-domain philosophy.
- Cliff/collision sensing layers and safety state machine direction.
- Parametric CAD, STEP/STL exports, CAD FMEA/requirements/validation dataset.
- A vendor-neutral firmware architecture scaffold now defines stable hardware
  interfaces, dependency registry, services, safe behavior commands, optional
  capabilities, and bench-minimal/stage1/full-prototype profile requirements.
- Host-side architecture tests cover profile validation, commissioning gating, and
  invalid high-level motion requests. Production hardware bindings are not implemented.
- Phase 2A adds source/session/sequence command identity, single-owner lease
  arbitration, legal motion-state transitions, duplicate/stale rejection, and local
  lease-expiry detection. It is not yet integrated with a physical safe-stop path.
- Phase 2B1 adds protocol-major compatibility, expected-peer and boot-session binding,
  ordered heartbeat liveness, explicit renegotiation, and fail-closed motion-authority
  removal. Hello alone grants no motion authority; each candidate/renegotiated session
  needs a fresh valid heartbeat. Stray/incompatible Hellos are rejected without
  destroying a healthy session. It has no wire codec, authentication, or physical
  transport binding yet.
- The first extensible-platform contract now centralizes versioned extension classes,
  categories, registry-assigned trust, typed permission requests, lifecycle/failure
  states, and declared/validated/active capability invariants. It is not yet a package
  schema, registry, loader, sandbox, or permission enforcement system.

## Current CAD snapshot
- Project/CAD v0.3: 39 automated checks; 38 pass; 1 restricted case.
- Dedicated VL53L1X head laser-rangefinder envelope added beside the centered camera.
- Estimated mass ~2.448 kg pending measured masses.
- Active modeled height ~282 mm; Shutdown modeled height ~182 mm.
- Two-arm simultaneous 250 g forward payload is not allowed during normal driving under the current support geometry.

## Main unresolved gates
See `OPEN_ITEMS_AND_FREEZE_GATES.md` and `OPEN_SOURCE_HARVEST_AUDIT.md`: display exact interface, laser optical-window/crosstalk + camera alignment, safe actuator commissioning, bus/cable fault containment, physical cliff testing, real arm/head mass and torque/thermal tests, wheel traction/tire choice, magnet force/temperature, battery protection sizing, audio/cable-life tests, and belly optical/thermal validation.

## Rule for future work
Every new answer/change must use the Memory/Inventory Loop + full engineering loop + Open-Source Harvest Loop, and update repository source-of-truth docs when approved.

## Next exact firmware step

Phase 2A and Phase 2B1 are committed and host validated. The implementation-dependent
Phase 2B2 path is `WAITING_FOR_VERIFIED_INPUTS`; its analysis in
`PHASE2B2_BUS_TIMING_ANALYSIS.md` blocks wire implementation until physical
timing, message inventory, priority, queue-latency, fragmentation, and security inputs
are verified. Complete those inputs and rerun priority-aware utilization/response-time
analysis before freezing serialization, integrity checks, or CAN/TWAI framing.
Production driver binding remains later work.
