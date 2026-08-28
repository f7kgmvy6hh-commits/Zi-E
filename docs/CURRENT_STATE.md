# Current State — 2026-08-28

## Phase
Prototype architecture and Project/CAD v0.3 package are prepared. Phase 2A now has a
transport-independent motion command lifecycle foundation plus a Phase 2B1
controller-session/liveness scaffold. Bounded wire encoding, transport binding,
production bindings, and bench commissioning remain next. Physical risk-retirement
bench rigs are still required before bulk component purchase.

## What is substantially defined
- A bounded in-memory package-policy foundation separates package/version/content
  identity, applies authoritative verification and compatibility, gates hosted
  activation, and models fresh-epoch update/rollback, watchdog, and secret policy.
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
- A separate device-identity contract now distinguishes extension package, physical
  hardware, logical instance, controller, and hardware-profile binding. It rejects
  ephemeral transport/location values as stable identity, represents hardware without
  a trustworthy serial as explicitly provisional/local, rejects collisions, preserves
  registry-assigned trust/ownership, and freezes identity after activation pending a
  future explicit migration/rebind workflow.
- The in-memory extension registry now binds manifests and device identities to
  externally assigned package/controller/profile/trust context, rejects duplicate or
  conflicting registrations, owns explicit lifecycle transitions, and resolves only
  validated active capabilities. Quarantine, disable, failure, inactivity, and removal
  synchronously revoke capability exposure. It does not load or execute extensions.
- Transactional configuration now separates declared, staged, validated, and active
  snapshots; binds requests to registry package/device/profile identity; rejects stale
  revisions and authority/raw-control namespaces; and preserves the prior active
  snapshot on every failed update. This is in-memory model state only and does not
  apply configuration to hardware.
- A semantic Robot API contract now separates authorized command intents, immutable
  event occurrences, and read-only versioned state. Commands require an active
  registry package/device provider and fixed active capability; protected safety and
  raw-control requests have no plugin path. Command sessions are explicitly bound by
  authoritative core context, replacement retires the prior session, and plugins
  cannot rotate a self-selected session to reset replay ordering.
- A bounded synchronous event bus now provides typed filtering, registry/capability
  subscriber checks, deterministic drop-newest backpressure, lifecycle delivery
  revocation, bounded retry, saturating per-subscriber dead-letter accounting, and
  callback-failure isolation. A deterministic virtual robot consumes
  only accepted semantic-command tokens and separates acceptance from simulated
  execution, authoritative state updates, and immutable result events.
- A vendor-neutral provider foundation now models LLM, STT, TTS, and wake requests and
  responses behind explicit semantic-capability routing and exact registry
  package/device/category/capability authorization. Ordered failover and typed
  diagnostic history are bounded, authorization is rechecked per attempt, and mocks
  cover malformed, throwing, revoked, mismatched, and exhausted providers.
- A bounded data-only presentation catalog validates and selects registry-owned face
  and sound packs per context. Declaration/validation is pack-global while activation
  is context-authoritative, supports shared selection, purges every affected selection
  on detected registry revocation, and requires explicit reactivation after recovery.
  Missing-context reads do not create state. The presentation engine consumes only accepted semantic
  expression/audio tokens, applies deterministic default/fallback/transition behavior,
  and updates state through authoritative core access. Virtual rendering is semantic
  only; no graphics, audio, asset loader, persistence, or hardware backend exists.

- A public in-memory Plugin SDK now defines explicit `1.0.0` contract-range
  compatibility, abstract host-issued immutable contexts, and narrow semantic command,
  event, read-only state, transactional configuration staging, provider invocation,
  and presentation-state services. The internal host issues package/device/profile/
  trust/capability identity and fresh registry-generation-bound activation epochs;
  initialization grants no services, callback exceptions are contained, and
  suspension/failure/quarantine/removal permanently revoke old handles.
  Asset packs and protected/embedded modules receive no executable context; provider/
  wake categories cannot receive motion; no raw-control or protected-safety SDK type
  exists. This is not a loader or runtime sandbox.
- Plugin contexts are explicitly immutable issuance snapshots. Current-context reads
  after suspend/failure/quarantine/recovery expose non-active metadata with no grants,
  removal exposes no context, and retained old services fail stale. Only explicit
  activation creates a fresh active context/epoch.
- An in-memory hardware-profile manager now validates versioned semantic hardware
  requirements and deterministically resolves them against exact registry-owned
  device/profile/category/lifecycle/capability identity. Optional absence is reported;
  missing required or ambiguous candidates fail activation. Active replacement is
  atomic, selected-device revocation invalidates the profile without silent rebinding,
  and explicit re-resolution/reactivation receives a fresh generation. Raw hardware
  and protected-safety bypass namespaces have no profile path. Discovery, parsing,
  drivers, commissioning, persistence, and physical validation remain deferred.
- An internal in-memory host runtime now orders fail-safe stopped startup, explicit
  profile resolution/activation, protected-safety checks, extension authorization,
  and provider/configuration/presentation/core-service readiness. Bounded immutable
  readiness snapshots distinguish required failure from safe optional degradation.
  Shutdown revokes extension epochs before profile authority. Versioned copied
  checkpoints contain semantic reconstruction intent only; recovery re-resolves the
  exact prior device set, revalidates selections, and requires explicit reactivation
  with fresh epochs. There is no persistence, scheduler, real hardware, or motion
  authority in this foundation.

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
