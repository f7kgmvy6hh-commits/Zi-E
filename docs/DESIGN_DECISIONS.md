# ZI-E Design Decisions — Concept v0.1

This document captures the design decisions made so far. Dimensions are not final unless explicitly stated.

## 1. Overall category and personality

- ZI-E is a **desktop companion robot**.
- It should not be extremely small or large.
- Exact size is secondary to good proportions, internal service space, battery capacity, and a clean desktop presence.
- Visual direction: **cute + futuristic + premium**, but believable and buildable.
- It should have its own identity rather than being a WALL-E or BMO copy.

## 2. Overall body shape

**Approved direction: Hybrid body.**

- Wider toward the bottom.
- Slightly narrower through the middle.
- Soft transitions toward the upper head opening.
- Rounded, non-boxy edges.
- Front chest is **gently convex** to give the robot personality.
- Rear remains sculpted and attractive; it must not look like a flat electronics access panel.

## 3. Head

### Shape
- Custom soft-trapezoid / futuristic head.
- Wider near the top and slightly narrower toward the bottom.
- Rounded edges.
- Balanced head size, not oversized-cute or tiny.
- Target proportion: roughly **55–60% of body width** as a concept guide.
- Head should be relatively low/wide to suit the landscape face and retraction.

### Face
- **One landscape digital screen.**
- Face is digital; no mechanical eyes or eyelids.
- Custom bezel around the display.
- Digital expressions can change without extra mechanics.

### Camera
- Camera belongs in the **top-center region above the screen**.
- It should be highly concealed.
- Preferred appearance: hidden behind a dark/smoked sensor window or very discreet slit.
- Camera remains usable in Active and Sleep modes.
- It becomes protected/hidden in full shutdown.

## 4. Head and neck motion

### Neck
- One central, short, relatively broad neck.
- Internal wiring should route through the neck where practical.

### Head motion
- Pan.
- Tilt.
- Vertical retract.

### Retraction mechanism
**Approved concept:** a vertical slider on internal guide rails.

Avoid a complicated telescopic neck for Version 1.

## 5. Operating poses

### Active Mode
- Head fully raised.
- Neck visible.
- Pan/Tilt available.
- Full visual interaction.

### Sleep Mode
- Head lowers enough for the neck to disappear.
- Display remains visible.
- Camera remains functional.
- Screen may show sleepy eyes, time, battery, or other low-energy information.
- Arms rest in a relaxed state.
- Hidden RGB heart/core uses a slower, dimmer pulse.

### Protected Shutdown
Goal: **maximum protection + minimum physical volume + attractive compact form**.

- Head retracts significantly deeper than Sleep.
- Screen and camera become physically protected.
- Head should be almost completely hidden inside the body.
- Top opening follows the head's shape.
- Body has a slight inward slope and a subtle protective rim around the head opening.
- Arms retract/fold into side recesses.
- Wheels can be manually tucked/folded inward.
- Robot should resemble a compact protected pod rather than a robot merely switched off.
- RGB heart/core turns off.

## 6. Top body opening

- Opening follows the custom head shape rather than being a generic rectangle.
- Upper surface slopes slightly inward toward the opening.
- Very subtle raised/protective rim.
- Aim for a nearly closed/flush-looking protected shutdown profile.

## 7. Chest light / “heart”

- RGB status/emotional light on the front chest.
- Location should be easy to see.
- **It must be hidden when off.**
- No obvious external heart badge when inactive.
- Preferred implementation concept: internal RGB light behind a flush, frosted/seamless diffuser integrated into the convex chest.
- Active: subtle expressive pulse.
- Sleep: slower/dimmer pulse.
- Shutdown: off.
- Future status colors may indicate charge/errors without making the chest look like a dashboard.

## 8. Drive system

### Approved base architecture
- Two **fixed powered wheels**, independently driven.
- Differential drive.
- Hidden passive support point/caster underneath.
- Zero-radius turning using opposite wheel directions.
- No swerve/360-degree steering in Version 1.
- No motorized ball drive.
- No Mecanum/omni base in Version 1.

Reasoning: sideways travel is not worth the extra mechanical complexity, size, wiring, backlash, and failure points for this companion robot.

The independently moving head can maintain gaze while the body reorients.

### Wheel appearance
- Semi-exposed.
- Roughly half visually integrated into the housing and half visible as a concept direction.
- Medium-to-large visual proportion, approximately **35–40% of body height** as a starting proportion only.
- Style: **hidden mechanical + futuristic**.
- Motors/axles/wiring remain concealed.
- Wheel pods should look designed into the body, not attached afterward.

### Protected shutdown wheels
- Manual inward tuck/fold is acceptable and desired.
- No powered wheel-fold mechanism for Version 1.
- Robot may rest on hidden rubber feet when wheels are tucked.

## 9. Arms

The arms are **functional and expressive**, not decorative only.

### Placement
- Shoulder mounts are high on the sides.

### Joints / functions
- Shoulder joint.
- Elbow joint.
- Telescopic forearm, one extension stage.
- Compact wrist/tool interface.
- Compact adaptive two-finger gripper as the normal/default hand.

### Arm reach
- Normal arm length should look proportional in daily use.
- Forearm extends only when extra reach is needed.
- Concept target: extension to roughly 135–150% of normal length, subject to engineering validation.

### Hand/gripper
- Two-finger adaptive gripper.
- Rounded and compact.
- Designed for light tasks such as:
  - holding small/light objects;
  - pressing buttons;
  - passing an object;
  - holding a stylus through an adapter;
  - pushing/pulling light items.

This is not intended as a heavy industrial manipulator.

### Wrist / tool ecosystem
- Manual quick-swap tool system for Version 1.
- Interface should be designed **auto-ready** for a possible future docking/tool-changing system.
- Future tool examples: stylus, sensor, magnetic pickup, suction tool, small light, custom tools.
- Mechanical lock, power/data contacts, and alignment concept will be engineered later.

### Arm shutdown sequence
1. Retract telescopic forearm.
2. Close gripper.
3. Return wrist to safe position.
4. Fold elbow.
5. Tuck shoulder.
6. Entire arm nests into a shaped side recess.

The recess should look like a natural body design line rather than an ugly cavity.

## 10. Internal service architecture

### Exterior vs structure
ZI-E uses:
- cosmetic outer shell;
- structural internal core/frame;
- modular internal subsystems.

The cosmetic shell should not be the sole load-bearing structure.

### Rear access
- Sculpted rear appearance.
- Rear shell should be removable for major service.
- No essential wiring should live on a removable cosmetic cover where it must be unplugged every time.
- Seams should follow design lines.

### Internal modularity
Use multiple modules placed in their optimal functional locations rather than one giant electronics tray.

Candidate zones:
- top: head lift / head motion;
- center: controller and communication;
- bottom-center: battery/power;
- sides: wheel modules and arm mechanisms;
- front peripheral: hidden RGB/core and forward sensors.

Modules should be removable independently where practical.

## 11. Battery concept

- Large battery capacity is a priority.
- Long runtime matters more than shrinking the body by a few centimeters.
- Battery bay should support later capacity upgrades.
- Battery lives low and central for a low center of gravity.
- Preferred access: **camera-style quick-release battery cartridge/door**.
- Best concept location: lower rear, on a subtly angled flush panel.
- User should not need to turn the robot upside down to access it.
- Door should not visually damage the rear design.

Battery chemistry/cell configuration is NOT yet selected.

## 12. Audio

- No ugly front speaker grille.
- Speaker should be hidden inside the body.
- Use an internal acoustic chamber and carefully designed vents/ducts.
- Consider symmetric output paths and desk-surface reflection.
- Do not assume “more holes = better sound”; final acoustic dimensions depend on the real speaker/enclosure.
- Keep speaker physically separated from the head microphone as much as practical.
- Software echo cancellation may be used later.

## 13. Exterior cleanliness

From the front, visible elements should be minimal:
- digital face;
- subtle camera location;
- hidden-when-off RGB heart/core;
- intentional body surfaces.

Speaker openings and sensors should be hidden in the best functional locations using vents, dark windows, seams, or lower/rear openings.

## 14. Serviceability principle

ZI-E should be upgradeable without redesigning the whole robot.

Examples:
- controller replacement;
- battery upgrade;
- wheel module replacement;
- arm/tool upgrade;
- speaker change;
- added sensor;
- future phone integration.

Prefer standardized internal mounting patterns and connectors.

## 15. Concept-image status

The repository contains concept sheets under `assets/concepts/`.

Important correction applied to later concept:
- camera made much less visible;
- RGB chest light made concealed/seamless when off.

Images are **visual direction references**, not dimensionally accurate CAD.


## 2026-08-23 — Hidden Belly Light Matrix and pre-purchase failure-harvest
**Decision:** Add a hidden RGB belly matrix behind a flush secret-until-lit optical insert. Prototype with IS31FL3741 13×9, but reserve larger custom matrix keepout in CAD. It is non-safety multimedia and must fail dark.

**Decision:** Add a permanent Open-Source Harvest Loop. Reachy Mini, LeRobot SO-101, OpenArm, Stretch, MoveIt/Nav2 and MuJoCo failure patterns are used to create preventive tests and architecture changes before bulk part purchase. Immediate changes include safe commissioning, explicit command execution state, single motion arbiter, bus/cable fault containment, dynamic payload/stability model, dynamic collision zones, diagnostic tooling and explicit simulation inertials.

## 2026-08-23 — Project v0.3 dedicated head laser rangefinder
- Add one dedicated forward single-point Class-1 ToF laser rangefinder in the head sensor brow.
- Primary candidate: VL53L1X, because up-to-4 m / 50 Hz is sufficient for a desktop companion and the final bare package is extremely small.
- Camera remains centered; rangefinder is offset and software must calibrate the camera/rangefinder extrinsic relationship.
- Rangefinder is for measurement/vision assistance and does **not** replace local STM32 cliff/proximity/bumper safety.
- The optical brow/window must be validated for ~940 nm transmission and crosstalk; arbitrary dark plastic is not accepted without test.
- TF-Luna becomes the intentional backup if >4 m range or >50 Hz is required; TFmini-S only if a real longer-range requirement appears.

## 2026-08-27 -- Modular hardware firmware boundary

- Preserve ESP32-S3 ownership of multimedia/presence hardware and STM32 ownership of
  safety/motion-critical hardware.
- AI/behavior uses a safe high-level command facade and never directly controls GPIO,
  PWM, registers, vendor APIs, motors, or individual actuators.
- Hardware is supplied through stable capability interfaces and explicit profile
  bindings. Optional hardware is represented as an absent capability, not hidden by a
  guessed or silent production fallback.
- Production motion remains gated by verified low-power commissioning and actual-state
  confirmation. The initial code is an architecture scaffold, not a hardware freeze.

## 2026-08-27 — Phase 2A motion lifecycle foundation

- Establish the command lifecycle as a transport-independent core before selecting a
  CAN/TWAI wire encoding.
- Identify motion commands by source, boot/session, and sequence; never use receipt
  order alone as proof of a new command.
- Permit one active motion command owner per subsystem. Duplicates are idempotent and
  do not renew leases; implicit replacement of an in-flight command is rejected.
- Check the monotonic lease on every state-changing call. Expiry is a local fault that
  requires a physical safe stop and later state confirmation by STM32.
- Defer cross-session authority negotiation, replay history, wire framing, heartbeat,
  and controller-reboot handling to Phase 2B.

## 2026-08-27 — Phase 2B1 controller session and liveness

- Negotiate controller compatibility by independently versioned protocol major/minor;
  a major mismatch fails closed and the negotiated minor is the lower supported value.
- Bind an active link to the configured peer controller and its nonzero boot session.
  A changed boot session removes motion authority and requires explicit local
  renegotiation; receipt of a new hello alone cannot silently replace the peer.
- A compatible expected-peer Hello establishes only a candidate session. Motion
  authority remains unavailable until the first valid bound heartbeat proves liveness;
  every renegotiated session requires a fresh heartbeat.
- Reject malformed, unexpected-controller, retired-session, and incompatible-version
  Hellos without mutating a healthy established session. Only a different boot session
  from the configured expected peer triggers a restart fault. This reduces trivial
  unauthenticated Hello denial of service without claiming authentication.
- Duplicate, stale, wrong-session, and invalid heartbeats do not renew liveness.
- Supply the monotonic heartbeat timeout from verified configuration. Do not invent a
  production timeout in reusable protocol code.
- Defer wire encoding, CAN/TWAI framing, integrity selection, and physical safe-stop
  binding until bus analysis and bench evidence exist. Long-lived authenticated replay
  protection remains explicitly open for Phase 2B2.

## 2026-08-27 — Extension manifest and capability trust foundation

- Treat `HOST_PLUGIN`, `ASSET_PACK`, `EMBEDDED_MODULE`, and
  `PROTECTED_SAFETY_MODULE` as separate execution models with class-specific validation.
- Assign trust from the registry/install source; never accept a manifest's claim that it
  is built-in, signed, or otherwise trusted.
- Treat requested permissions as declarations for a future deny-by-default enforcement
  layer, not grants. No raw actuator/safety-bypass permission exists; host motion access
  means Safe Robot API requests only.
- Keep declared, validated, and active capabilities separate. A manifest claim cannot
  populate either validated or active state, and inactive lifecycle states expose none.
- Version extension package, Plugin API, and manifest schema independently.
- Keep the current implementation as an in-memory validation contract. Parsing,
  loading, sandboxing, signatures, registry state transitions, and configuration
  activation require later reviewed foundations.

## 2026-08-27 — Device identity foundation

- Keep extension/package identity, physical manufacturer/model/serial identity,
  registry-generated logical instance identity, controller identity, and hardware
  profile binding as separate typed domains.
- Never derive stable identity from display name, USB path, COM port, IP address, CAN
  address, discovery order, or another transport/location observation.
- Represent devices without trustworthy serials using explicit registry-generated
  provisional-local identity and provisional trust. Promotion/rebinding is deferred to
  a future explicit migration workflow.
- Fail closed on duplicate physical or logical identities, conflicting provenance,
  unknown enum values, plugin-assigned controller/profile bindings, or changes after
  activation.
- Reserve protected STM32 safety-controller identity for registry assignment to
  built-in-trust extensions. Identity does not confer capabilities or permissions.
- Treat owning package, controller assignment, hardware-profile assignment, and
  extension trust as external registry validation context. Candidate records carry no
  authority flag and must exactly match that context.

## 2026-08-27 — In-memory extension registry and activation

- Accept an extension record only after manifest and device-identity validation under
  external registry assignment of package ID, trust, controller, and hardware profile.
- Reuse the shared lifecycle vocabulary and permit only explicit transition edges.
  Invalid/unknown transitions do not mutate registry state.
- Preserve removed records as in-memory tombstones. Duplicate packages and conflicting
  physical/logical identities fail closed.
- Resolve a capability only from active/degraded records after declared-to-validated-to-
  active subset checks. Reject activation when another live provider would make
  ownership ambiguous.
- Clear active capabilities synchronously on inactivity, failure, quarantine, disable,
  or removal. Capability ownership includes package and logical device instance.
- Registry acceptance grants no permission and exposes no raw hardware-control path;
  plugin motion remains semantic Safe Robot API requests only.
- Forbid generic lifecycle transition into `ACTIVE`. Both initial activation and
  degraded recovery must enter through capability activation after explicit
  inactive/activating states.

## 2026-08-27 — Transactional configuration foundation

- Separate declared, staged, validated, and active configuration. A failed candidate
  never mutates the prior active snapshot; rollback removes only staged/validated work.
- Bind every request to external authoritative package, logical device instance, and
  hardware-profile identity. Reject stale/nonzero revisions; assign a new registry
  generation only on successful commit.
- Permit commit only for inactive, active, or degraded owners. Quarantined, disabled,
  failed, activating, installed, and removed extensions cannot activate configuration.
- Configuration cannot grant permissions, trust, capabilities, controller/profile
  ownership, or safety authority. Reject raw GPIO/PWM/register/actuator/driver/safety
  bypass/CAN namespaces; hardware-affecting values remain semantic only.
- Require explicit inclusive signed 64-bit bounds for every bounded integer. Parse
  exact base-10 text without coercion; accept booleans only as lowercase `true` or
  `false`.

## 2026-08-27 — Semantic Robot API and command/event/state separation

- Define plugin-facing operations as typed semantic intents with package/device,
  session, and sequence identity. Map each command type to a fixed active capability;
  callers cannot choose their authorization requirement.
- Accept commands only from exact active registry providers. Quarantined, disabled,
  removed, failed, degraded, or inactive issuers cannot submit. Protected STM32 safety
  operations remain internal.
- Treat events as immutable occurrence records with no command or state side effect.
  Treat state as generation-ordered read-only snapshots written only through an
  authoritative core path.
- Keep raw motor setpoints, PWM, GPIO, registers, actuator ownership, raw CAN frames,
  and safety bypass outside the semantic command vocabulary.

## 2026-08-27 — Authoritative sessions, resilient events, and virtual robot

- Treat a command-carried session number as an identifier, never as authority. Only an
  authoritative core operation can bind or replace the active session for an exact
  registry package/logical-device pair; replacement permanently retires the prior ID.
- Keep replay ordering per authoritative session. Unknown, zero, mismatched, retired,
  duplicate, and stale session/sequence combinations fail closed.
- Bound every subscriber queue and expose deterministic `drop_newest` overflow instead
  of allowing silent memory growth. Recheck lifecycle/capability eligibility at publish
  and delivery, and isolate callback exceptions from other subscribers.
- Allow the virtual robot to consume only opaque accepted-command tokens. Acceptance,
  simulated execution, authoritative state mutation, and immutable event publication
  remain distinct steps; simulated failures preserve relevant state.
- Keep this host-only model outside physical safety claims. STM32 interlocks,
  commissioning, drivers, actual-state confirmation, scheduling, transport, and CAN
  remain separate future work.

## 2026-08-27 — Event delivery repair and provider resilience foundation

- Retain a failed event at the head of its subscriber queue for a configured bounded
  number of delivery attempts. Dead-letter it exactly once at exhaustion and expose a
  saturating per-subscriber count; never consume another subscriber's copy.
- Define LLM, STT, TTS, and wake contracts as vendor-neutral semantic values. Real
  networking, secrets, audio data/drivers/codecs, and provider SDKs remain excluded.
- Bind every provider to an exact registry package, logical device, manifest category,
  and unique kind-prefixed active capability. Recheck authorization at every call so
  lifecycle or capability revocation fails closed.
- Preserve unambiguous exact-capability ownership with distinct provider-instance
  capabilities. Provider order is explicit, provider count and attempts are bounded,
  and malformed responses or exceptions cannot bypass validation or extend failover.
- Provider outputs confer no Robot API, state mutation, safety, hardware, or raw
  actuator authority.

## 2026-08-27 — Typed provider routing and data-only presentation packs

- Require every provider invocation to name an exact semantic capability. Separate
  shared semantic routing capability from each provider's unique registry capability,
  preserving both deterministic failover and unambiguous registry ownership.
- Preserve bounded typed diagnostics for temporary/permanent failure, malformed
  response, provider exception, authorization loss, and capability mismatch. A
  successful fallback retains earlier failures; exhaustion retains its final cause.
- Keep face and sound packs data-only `asset_pack` records with authoritative package,
  version, logical-device, profile, schema, category, lifecycle, and capability checks.
  Packs cannot declare executable entrypoints, permissions, trust, or hardware control.
- Permit one selected face and one selected sound pack per presentation context.
  Replacement is deterministic and every use rechecks lifecycle/capability revocation.
- Route only accepted semantic expression, speech, and cue tokens into the presentation
  engine. State changes use the authoritative core; display/audio implementations and
  all raw hardware interfaces remain outside the layer.

## 2026-08-27 — Context-authoritative pack activation correction

- Keep pack declaration/validation state global to the data record and keep activation
  exclusively in bounded per-context selections. A pack may serve multiple contexts;
  replacing one selection cannot alter another.
- Make missing-context queries read-only and fail closed. When use-time registry
  reauthorization detects revocation, purge the affected pack from every context.
- Require explicit contextual activation after valid registry lifecycle recovery;
  never infer or restore selection from global pack state.

## 2026-08-28 — Plugin SDK and stable extension host boundary

- Publish only independent SDK DTOs and abstract semantic service/context interfaces.
  Keep the extension host and `AuthoritativeRobotCore` internal.
- Negotiate an explicit inclusive SDK contract range against the host-supported range.
  Reject zero/reversed ranges, incompatible majors, unsupported future versions, and
  unknown domains/capabilities; never infer future compatibility.
- Issue identity, trust, profile, capabilities, command session, sequence, and
  activation epoch from the host. Extension initialization receives no active
  capabilities or services.
- Reauthorize every SDK call against the live registry and fixed service capability,
  then route through the existing subsystem checks. SDK adapters confer no extra
  authority.
- Invalidate an activation epoch before suspension, failure, quarantine, or removal.
  Recovery returns only to inactive and requires a new activation/context/epoch.
- Bind each SDK epoch to a monotonic registry authorization generation that changes on
  activation and every revocation. Contain all extension callback exceptions; a
  throwing activation is failed and cannot leave usable handles.
- Revoke every issued context and tracked event subscription when the extension host
  is destroyed, so a retained adapter cannot outlive host authority or dereference a
  released backend.
- Supersede the earlier permanent package-tombstone rule narrowly: explicit full
  re-registration with a new logical instance identity may replace a removed registry
  record, but never revive it. Old host objects, contexts, service handles, epochs, and
  command sessions remain retired.
- Give executable SDK contexts only to host plugins. Asset packs stay data-only;
  embedded/protected modules stay outside the third-party host; provider/wake
  categories cannot receive semantic motion; no protected-safety or raw-control SDK
  domain exists.
- Expose configuration only as copied reads and typed stage requests with a host-built
  authoritative binding. Validation/commit remains transactional core authority, and
  identity/authority/raw-control namespaces remain forbidden.

## 2026-08-28 — Context snapshots and authoritative hardware-profile resolution

- Define every Plugin SDK `ExtensionContext` as an immutable issuance snapshot.
  Retained snapshots may preserve historical metadata, but their handles are
  epoch-bound and fail stale after revocation. The host's current-context query
  replaces active metadata with a zero-grant inactive/failed/quarantined snapshot,
  returns null after removal, and issues a fresh context only after explicit
  activation.
- Treat profiles as trusted, versioned semantic requirement sets resolved only
  against registry-authoritative device identity and lifecycle. Resolution uses exact
  hardware-profile binding, category, active validated capability, protected
  ownership, logical device, and authorization generation; ambiguity fails closed.
- Make activation atomic. Revocation invalidates the selected result and no spare is
  silently rebound; explicit re-resolution and reactivation are required.
- Exclude transport identity, GPIO/PWM/register/setpoint data, raw actuator control,
  and safety bypasses from profile content. Discovery, parsing, drivers,
  commissioning, persistence, and transport remain deferred.

## 2026-08-28 — Authoritative host runtime and safe recovery intent

- Keep orchestration internal and downstream of every existing subsystem authority.
  Startup establishes stopped semantic state, then orders profile, protected safety,
  extensions, and optional services. No coordinator call bypasses subsystem checks.
- Derive requiredness only from the trusted startup plan and fixed core-domain policy.
  Required or protected failure fails the runtime; explicitly optional failures may
  degrade it without granting authority.
- Revoke hosted extension epochs in reverse startup order before deactivating profile
  authority on shutdown or required failure.
- Treat checkpoints as bounded copied intent, never authority. Exclude sessions,
  handles, leases, setpoints, heartbeat state, command sequence authority, and safety
  bypass state.
- Split recovery into explicit prepare and complete phases. Re-resolve and compare the
  exact prior device set before profile/extension activation; revalidate configuration
  generations and presentation selections; reject substitution and bound retries.

## 2026-08-28 — Package security and update authority

- Keep package ID, version, and content identity distinct; transport/path is never
  identity and package declarations never grant trust or protected ownership.
- Updates and rollback traverse registry generation replacement and ordinary
  ExtensionHost activation, issuing fresh epochs and never resuming motion.
- Protected safety remains built-in-only; asset packs remain data-only; capability
  expansion needs explicit registry validation.
- Watchdog output is semantic only and plugin credentials are opaque owner handles.

## 2026-08-28 — Final software foundation freeze candidate

- Freeze one authority chain: package policy establishes eligibility; registry owns
  identity/trust/capabilities; profile manager owns exact resolution; ExtensionHost
  owns executable epoch/context/session issuance; HostRuntime only coordinates.
- Keep LLM, STT, TTS, and Wake as independent modality chains with exact semantic
  capabilities, authoritative ordered priority, bounded typed failover, and current
  registry authorization on every attempt. A provider cannot self-promote and a
  temporary fallback does not alter future primary eligibility.
- Freeze VirtualRobot only as the software integration contract. It cannot satisfy a
  physical commissioning gate or prove that an accepted command executed in hardware.
- Freeze the future App as a consumer of public/semantic services. It cannot bypass
  HostRuntime, registry, ExtensionHost, profile, package, or protected-safety checks.
- Adopt the Prototype P1 commissioning progression and require separate operator/
  physical evidence for every physical transition. Autonomous motion remains outside
  P1 and prohibited before all later safety gates.
- Keep all CAN/wire values and received-part-dependent pins, interfaces, dimensions,
  loads, timing, electrical limits, and physical performance explicitly unfrozen.

## 2026-08-28 — Zi-E V1 local command-center boundary

- Implement the laptop command center as a thin authenticated operator/UI layer around
  one persistent Hermes session; it is not another agent, tool loop, memory authority,
  or provider credential owner.
- Bind only to validated loopback and require bearer authentication for every API and
  event connection. Never expose Hermes, OAuth, ElevenLabs, or provider secrets to the
  browser; structurally redact logs and cloud-bound speech text.
- Expose narrow health/state/settings, Hermes chat, semantic robot-state, E-stop,
  voice-control, and event surfaces. A HUD panel does not imply a generic terminal,
  browser, filesystem, or actuator API.
- Keep robot state deterministic and fail closed. E-stop is reachable from every
  state; active host modes have a dead-man deadline; every command records source,
  UTC timestamp, semantic target, timeout, and result. Host acceptance is never
  physical execution confirmation.
- Default to real/non-simulator mode with null robot telemetry. Simulator data must be
  explicitly enabled and labeled and cannot satisfy commissioning evidence.

## 2026-08-28 — Zi-E V1 Hermes and voice execution

- Invoke Hermes 0.20.6 as `chat --continue <stable-name> --create-if-missing -q
  <message> -Q --source tool` and preserve one named session.
- Superseded: prompt-complexity routing between MiniMax and OpenAI is not provider
  authority. Send ordinary requests to the configured LLM primary (defaulting to the
  OpenAI-capable Hermes model), and let explicit `/model` requests bypass App
  selection. Bounded fallback remains modality-specific HostRuntime policy;
  `smart_model_routing` is only a setup placeholder and is not consumed.
- Stream redacted TTS in bounded chunks, centralize ElevenLabs voice/model, retain
  Zi-Nanami fallback, and cancel remaining work wherever the generator can observe it.

## 2026-08-28 — Current App/STT reconciliation

- Treat the current Python command center as a simulator/operator prototype, not a
  HostRuntime implementation or physical authority. A real target stays unavailable
  until the App consumes HostRuntime/public semantic services and commissioning gates.
- Keep OpenAI-capable providers preferred when configured and authorized, separately
  for LLM, STT, TTS, and Wake. Existing local Whisper, ElevenLabs, and Zi-Nanami
  adapters are fallback/available implementations; they cannot set authoritative
  priority or widen capabilities.
- Isolate native local Whisper execution in a bounded subprocess with a strict typed
  result contract. Ordinary STT/TTS never depends on Codex development authority.
- Fail closed when the App is configured for a real target without a commissioned
  HostRuntime/public-semantic adapter: retain `DISCONNECTED`, reject ordinary state or
  motion commands, and represent E-stop only as `requested_not_delivered`. A config
  flag cannot create physical authority or execution confirmation, and no event may
  label an unavailable target as physical.

## 2026-08-28 — App semantic HostRuntime adapter and honest status freeze

- Make `HostRuntimeAdapter` the App's only future real-target boundary. Its surface is
  copied status, closed semantic command request/response, semantic E-stop request,
  and polling; it contains no raw hardware or authority-mutation method.
- Distinguish simulation, real-target-unavailable, and reserved future-real-target.
  Bind every future-real response to current adapter target mode, positive authority
  generation, and session; a mid-request change fails closed before execution events.
- Distinguish requested, accepted, rejected, and confirmed phases; delivery and
  physical confirmation are independent. Only runtime feedback may assert physical
  confirmation. Simulation never does.
- Expose profile/configuration/presentation/provider/extension visibility honestly.
  Missing capability-scoped HostRuntime operations are `not_exposed` or unavailable,
  never decorative authority. Do not silently rebind devices or profiles.
- Keep App provider status separate for LLM/STT/TTS/Wake, exact-capability and ordered.
  Configuration is not authorization; report `not_verified` until a live HostRuntime
  check exists. Fallback is bounded and non-sticky in deterministic contract tests.

## 2026-08-28 — Zi-E HUD plugin presentation

- Use an original cinematic robotics-control visual language: graphite surfaces,
  cyan/ice-blue information, amber caution, and red emergency states. Do not copy
  Marvel/JARVIS branding, assets, phrases, or layouts.
- Treat HUD panels as host-registered plugins with declarative identity, permission,
  status text, and event subscriptions. The current registry is intentionally a
  lightweight browser presentation layer, not a privileged executable sandbox.
- Keep privileged actions behind the authenticated Zi-E backend. Plugin panels cannot
  access Hermes credentials, provider keys, arbitrary environment values, raw actuator
  control, or safety bypasses.
- Reserve port `8765` for the existing Zi-Nanami service and run Zi-E on loopback
  port `8766`.
- Enable `ZIE_SIMULATOR=true` for current host-only integration testing. Simulator
  telemetry is not physical evidence and must remain clearly labeled.
