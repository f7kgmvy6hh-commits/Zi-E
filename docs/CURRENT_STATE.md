# Current State — 2026-08-30

## Development pipeline and physical integration preparation — 2026-08-30

- Host toolchain is ready through fixed detected paths: Git 2.45.1, project Python
  3.11.16, uv 0.12.7, CMake 4.3.3, Ninja 1.13.2, MSYS2 UCRT64 GNU C/C++ 16.1.0,
  Windows PowerShell 5.1, Node 26.7.0 and Codex CLI 0.151.0. The validated venv has no
  pip module/executable but its App dependencies import; the existing uv-based setup is
  the rebuild path.
- Espressif EIM, ESP-IDF/idf.py, ESP32-S3 compiler, esptool and OpenOCD are not
  installed/detected. WinGet execution is sandbox-blocked, so official installation is
  a manual host action. Exact ESP-IDF version remains unknown, not inferred.
- STM32CubeCLT, GNU Arm GCC/GDB, STM32CubeProgrammer, ST-LINK tools/drivers and CubeMX
  are not installed/detected. ST license/installer acceptance remains a manual action.
- The repository now has a toolchain doctor, one fixed firmware workflow, and disabled
  ESP32/STM32 build boundaries. Target operations require an explicit reviewed board
  profile; flash/program also require explicit port/probe identity. No profile exists,
  no fake pins/clock/CAN values were added, and no physical transmit path is active.
- Bench 0 (PC, App and host firmware) is executable. Bench 1+ remains blocked on vendor
  tools and verified physical targets. Hardware is not verified, first power is not
  authorized, physical commissioning is not done, and Phase2B2 remains
  `WAITING_FOR_VERIFIED_INPUTS`. See `DEVELOPMENT_PIPELINE_READINESS.md`.
- Fresh validation: 120 App pytest tests, 3 toolchain/workflow tests, 44 fallback tests,
  host CMake/Ninja build (35 steps) and CTest 1/1 pass. Python compileall, PowerShell,
  HTML and JavaScript parsing, doctor smoke check and `git diff --check` pass. ESP-IDF
  and STM32 compile smokes are unavailable because their official tools are absent;
  no hardware test or flash/program action was performed.

## First real bench evidence closure — 2026-08-30

- Derived first-bench status is `EVIDENCE_COLLECTION_ONLY`: unpowered article/label/
  connector inspection and measurement may proceed, plus narrowly bounded continuity
  or host-only cable checks after instrument identity is reviewed. No received active
  module is presently approved for power.
- `HW-010` is `DO_NOT POWER OR CONNECT`; its input/output, polarity, isolation,
  current, protection/fuse and connector evidence is incomplete. No other regulator is
  owned; `HW-033` remains candidate-only.
- `HW-002` remains ordered. Delivered board/revision, ESP32 module, camera sensor/FPC,
  USB arrangement and physical pinout evidence are required before camera testing.
  Workbook wiring/GPIO/firmware maps remain provisional and freeze no pin or harness.
- Received ESP32-side peripheral candidates have item-specific evidence closures and
  conditional independent tests, but ownership/profile binding is not inferred and no
  powered test is yet authorized.
- STM32, motion, battery/BMS and CAN hardware/evidence remain absent. First power is
  unauthorized, commissioning has zero passes, robot authority is `NONE`, and Phase2B2
  remains `WAITING_FOR_VERIFIED_INPUTS`. See `FIRST_BENCH_READINESS.md`.

## Real hardware evidence intake — 2026-08-29

- The 0.04 engineering working copy now contains 31 supported rows from
  `Zi-E_Hardware_Master_Inventory.xlsx`, source SHA-256
  `378b06f0f34baa188b7b496d3e0a7f0e12e1b043fe082e0ca83a23e3685660b6`:
  167 total units, 29 `RECEIVED`, one `ORDERED`, and one physical status `UNKNOWN`.
- Every imported row remains `REVIEW_REQUIRED`, `UNDECIDED`, and not verified.
  `HW-032` and `HW-033` remain seller-reference candidates and are excluded from
  purchased inventory.
- `HW-002` remains ordered and blocked on physical board/revision/sensor/pinout
  comparison. Workbook camera/GPIO/wiring/firmware sheets remain provisional plans,
  not physical evidence or frozen pins.
- `HW-007` VL53L0X-family hardware and `HW-008` WS2812/WS2811-family breakouts are
  explicit mismatches against the different rangefinder and belly-matrix candidates;
  both require an explicit KEEP/REPLACE decision. `HW-010` remains blocked from system
  power pending electrical verification. The multimeter receipt is retained while its
  physical receipt remains unresolved.
- Required-hardware coverage remains unreviewed; STM32, CAN/controller-link, motion,
  drive, battery/BMS, protected power, and other absent robot hardware remain missing.
  CAD and electrical evidence remain incomplete, ESP32/STM32 remain waiting for
  evidence, first power is unauthorized, all physical commissioning gates remain
  unpassed, robot authority remains `NONE`, and Phase 2B2 remains
  `WAITING_FOR_VERIFIED_INPUTS`.

## ZI-E Control Center 0.04 — 2026-08-29

- Pushed checkpoint `680bb9f` contains Control Center 0.04 and the real inventory
  intake. Current preparation remains uncommitted by request.
- App version `0.04` reflects API/HUD changes. Physical evidence entry alone does not
  require a software-version bump.
- Active working inventory contains 31 evidence-supported lines; a newly initialized
  store remains empty by default. Design candidates stay separate and are never seeded,
  received, or verified automatically.
- Required Hardware, WHAT I NEED FROM YOU, CAD/electrical/driver, ESP32/STM32,
  first-power, commissioning, and Phase 2B2 reports are conservative, grant robot
  authority `NONE`, and pass zero physical gates.
- Work may branch by evidence readiness. Phase 2B2 gates production controller-link/CAN
  decisions specifically, not every independent device driver.
- Fresh 0.04 validation: 113 App pytest tests and 44 fallback tests pass. Compileall,
  PowerShell parsing, HTML/JavaScript parsing, the 32-column schema and reconciliation
  invariants, and `git diff --check` pass. Independent review findings were repaired:
  suggestions cannot satisfy coverage; unreviewed text cannot advance engineering
  readiness; KEEP requires identified reviewer/date and reviewed evidence.

## ZI-E Control Center 0.03 — 2026-08-29

- Product-facing App version is `0.03`: hardware inventory, evidence, and
  reconciliation foundation. The staged 0.02 work remains preserved beneath this
  stage. It was committed in pushed base `214ca2d`.
- The exact 32-column purchased-parts CSV remains the compatibility schema. New
  lifecycle/revision fields live in a separate application-owned JSON store under
  ignored `runtime/inventory/`, preventing silent CSV schema drift.
- Inventory starts empty. Users can create/edit bounded intake fields, tombstone
  records, preview/confirm canonical CSV imports, export deterministic CSV, search,
  filter, sort, inspect engineering detail, record explicit reviews/decisions, and
  request verification only through a guarded review transition.
- Writes are UTF-8, bounded, validated, atomic, optimistic-revision protected, and
  preserve one validated previous revision for fixed rollback. Malformed active data
  fails closed. Browser requests never select paths, executable content, or generic
  JSON/filesystem operations.
- Reconciliation exposes identity, evidence, candidate-match, ownership, logical
  slot/Profile, driver, CAD, measurement, electrical, safety, decision,
  commissioning-dependency, blocker, and Phase 2B2 states without inventing parts or
  measurements. Design candidates are never imported as purchased inventory.
- Inventory, evidence, review, import, tombstone, decision, and rollback operations
  have robot authority `NONE`; they cannot pass any of the 16 physical commissioning
  gates. Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`.
- Historical 0.03 validation: 104 App pytest tests and 44 fallback tests passed; compileall,
  PowerShell, HTML, JavaScript, canonical 32-column schema/migration validation, and
  `git diff --check` passed. Independent adversarial findings around conflict-safe
  verification, lifecycle revocation, CSV enum rejection, concurrent revisions, and
  blocker navigation were repaired and regression-tested.

## ZI-E Control Center 0.02 — 2026-08-29

- Product-facing App version is now authoritatively `0.02`; `0.01` names the prior
  structural Project Cockpit foundation. Protocol, SDK, CAD, and package versions
  retain their separate technical meanings.
- The 16-workspace cockpit now provides a generated static camera test source,
  semantic face/RGB preview controls, bounded simulation-only drive requests with a
  500 ms server-side dead-man expiry, and
  one-at-a-time actuator commissioning previews. Every preview reports simulated or
  not-delivered state and can never report physical confirmation.
- Sensor, power, controller, controller-link, inventory, commissioning, and safety
  views remain structurally useful while preserving null/unavailable physical data.
- Developer actions remain a server-owned closed allowlist. Repository, Windows
  Terminal, Codex, and contained log launchers use fixed repository-derived paths and
  fixed arguments when local executables are available. Hermes is available only for
  the exact existing `hermes` configuration; otherwise it remains unavailable.
- No production hardware driver, CAN transmit, raw actuator/GPIO/PWM, generic shell,
  arbitrary filesystem, unrestricted flashing, or physical commissioning path exists.
  Autonomous physical motion remains blocked and Phase 2B2 remains
  `WAITING_FOR_VERIFIED_INPUTS`.
- Fresh validation: 76 App pytest tests and 44 fallback tests passed; Python
  compileall, PowerShell parsing, HTML parsing, JavaScript syntax checking, and
  `git diff --check` passed. Independent adversarial findings were repaired: drive
  expiry/release, visible physical blockers, honest detached-launch acceptance, and
  mismatched actuator-stop rejection.

## Control Center 0.01 cockpit — 2026-08-28

- The authenticated loopback App now has 16 workspaces for operator status, simulator
  semantics, hardware intake, P1/CAD, commissioning, safety, providers/plugins,
  diagnostics and development.
- A consolidated honest model reports ESP32/STM32 ownership and unavailable hardware
  fields without fabricated telemetry. Inventory previews remain
  `NOT_VERIFIED`/`REVIEW_REQUIRED` with no persistence or device/profile binding.
- Developer actions are fixed authenticated identities with bounded execution/output
  and `robot_authority: NONE`. No generic terminal, filesystem, raw CAN/GPIO/PWM,
  flashing, or direct actuator endpoint exists.
- Physical commissioning is `NOT DONE`, production drivers `NOT STARTED`, and Phase
  2B2 `WAITING_FOR_VERIFIED_INPUTS`.

## STT isolation validation — 2026-08-28

- Local `faster-whisper` is installed in Zi-E's project environment and the `base` model is cached locally.
- A native crash was observed during direct model execution; therefore Whisper is not executed in the Zi-E server process.
- `POST /api/stt/transcribe` now launches `app.voice.stt_worker` as an isolated subprocess with a bounded timeout.
- Worker failures are reported as `STT_WORKER_CRASHED` or `STT_WORKER_TIMEOUT`; the main server remains isolated from native STT failures.
- Browser push-to-talk uses `MediaRecorder` and uploads only after user release; microphone permission is user-controlled.
- Empty-audio endpoint regression coverage passes without loading the Whisper model.

## App real-target fail-closed correction — 2026-08-28

- The current Python App has no commissioned HostRuntime/public-semantic real-target
  adapter. With simulator mode disabled, its robot remains `DISCONNECTED`; ordinary
  commands are rejected as `rejected_real_target_unavailable`.
- The operator E-stop surface returns HTTP 503 with `requested_not_delivered` without
  mutating local state or claiming physical execution. State events use
  `real-target-unavailable`/`not_delivered`, never `physical` based only on config.
- Health, state, and plugin surfaces distinguish simulation from unavailable real
  target. Future confirmed real execution must come from the commissioned adapter and
  cannot be synthesized by this App state machine.
- Validation: full App pytest passed 41 tests; fallback regression runner passed 31;
  Python compileall, PowerShell parsing, and `git diff --check` passed.

## Phase
Zi-E V1 now has a committed local command-center foundation under `app/` plus the
current App-completion candidate: loopback-only authenticated FastAPI, bounded
WebSocket events, central state,
redacted JSONL audit logging, real host metrics, a responsive graphite/cyan HUD,
fail-closed semantic robot state/dead-man/E-stop handling, modular cloud/command voice,
and a thin persistent-session Hermes executable contract. It changes neither Hermes
nor firmware and adds no physical robot transport. See `COMMAND_CENTER.md` and
`APP_HOSTRUNTIME_ADAPTER_CONTRACT.md`.

Software Foundation Freeze and Prototype P1 Build-Ready package remain `READY`.
Prototype architecture and Project/CAD v0.3 are prepared. Phase 2A has a
transport-independent motion command lifecycle foundation plus a Phase 2B1
controller-session/liveness scaffold. Bounded wire encoding, transport binding,
production bindings, and bench commissioning remain next. Physical risk-retirement
bench rigs are still required before bulk component purchase.

The integrated host authority chain, end-to-end VirtualRobot contract, independent
LLM/STT/TTS/Wake priority policy, App handoff boundary, and commissioning gates are
documented in `SOFTWARE_ARCHITECTURE_FREEZE_CANDIDATE.md`,
`FINAL_SOFTWARE_INTEGRATION_AUDIT.md`, and `PROTOTYPE_P1_BUILD_READY.md`. This status
does not claim hardware commissioning or freeze any unverified physical value.

## Prototype P1 source-of-truth reconciliation — 2026-08-28

- Software Foundation and App Software Foundation remain `READY`.
- P1 build-ready documentation now has a canonical component reconciliation, empty
  purchased-parts/Google Sheet intake contract, CAD freeze candidate, assembly and
  electrical preparation, ordered first-power package, arrival measurement worksheet,
  and condensed Phase 2B2 gate.
- No current component is marked physically `VERIFIED`. Current choices are documented
  candidates, undecided implementations, or `VERIFY_ON_ARRIVAL`; reviewed evidence in
  the Control Center working copy and an explicit project snapshot/export establish the
  project inventory record. A Google Sheet remains intake convenience only.
- Current CAD remains v0.3: internally automated geometry evidence is preserved, while
  every received-part-dependent dimension, mass/COM, torque/current/thermal, traction,
  sensor, cable and fit result remains parametric or blocked by measurement.
- Physical commissioning is `NOT DONE`. Production drivers are `NOT STARTED` and
  blocked only where their exact physical/profile inputs are required. Phase 2B2 remains
  `WAITING_FOR_VERIFIED_INPUTS`.
- P1 build-ready documentation is `READY` for arrival-day evidence intake and
  controlled bench execution. This is process readiness only, not physical readiness.
- P1 artifact validation: the committed CAD dataset/schema reports 39 checks, 38
  passing and one restricted; the empty purchased-parts template passes its 32-field
  schema. Fresh CAD regeneration is unavailable in this environment because CadQuery
  is not installed, and the release checksum manifest disagrees with an untouched STEP
  checkout file, so no fresh geometry or full artifact-integrity claim is made.

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
- Provider priority is explicit, monotonic, authoritative configuration per exact
  modality/capability chain. Providers cannot self-promote; each bounded attempt
  rechecks registry authority, and temporary fallback never locks out the primary.
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

## Zi-E V1 command center — 2026-08-28 continuation

- Hermes 0.20.6 now runs directly with one named session, deterministic routing,
  explicit `/model` bypass, and secret-free route/tool/response events.
- Voice speak streams redacted ElevenLabs chunks with centralized voice/model config,
  cancellation signaling, and Zi-Nanami fallback.
- Exact lifecycle scripts optionally own Camofox, start and health-check the server,
  open the HUD, and stop only revalidated owned PIDs.
- Example YAML and all requested operator docs are present. No motion authority was
  added. Full pytest passed 28 tests without warnings; PowerShell parsing and Python
  compileall passed. The tracked archived `.pyc` is deleted in the working tree.

## Next exact firmware step

Phase 2A and Phase 2B1 are committed and host validated. The implementation-dependent
Phase 2B2 path is `WAITING_FOR_VERIFIED_INPUTS`; its analysis in
`PHASE2B2_BUS_TIMING_ANALYSIS.md` blocks wire implementation until physical
timing, message inventory, priority, queue-latency, fragmentation, and security inputs
are verified. Complete those inputs and rerun priority-aware utilization/response-time
analysis before freezing serialization, integrity checks, or CAN/TWAI framing.
Production driver binding remains later work.

## Final integration validation

- Fresh MSYS2 UCRT64 GNU g++ 16.1.0 C++17 configure/build: passed.
- Full CTest: 1/1 passed.
- Separate fresh `-Wall -Wextra -Wpedantic -Wswitch-enum` configure/build: passed
  without warnings.
- Strict CTest: 1/1 passed.
- `git diff --check`: passed (Git emitted only line-ending conversion notices).
- Verified base/unchanged HEAD: `da5143b83bacf33a5afcf99ab98ab17b2c694147`.

## V1 continuation validation — 2026-08-28

- Zi-E runs on loopback port `8766`; the existing Zi-Nanami service remains on
  `8765`.
- Hermes `0.20.6` is connected through the installed executable and a real chat
  request succeeded through `/api/chat`.
- Simulator mode is enabled for safe host-only testing; no physical transport is
  enabled.
- ElevenLabs Sarah voice with `eleven_flash_v2_5` returned real audio through the
  Zi-E voice endpoint. The local Zi-Nanami command remains the fallback path.
- The HUD now has an original cinematic Zi-E robotics presentation, functional
  chat/control handlers, live event connection support, and an external plugin
  registry served under `/hud/`.
- Lifecycle scripts were validated through start, health, stop, and restart.
- Full local fallback suite: 24 passed, 0 failed.

## Voice fallback validation — 2026-08-28

- ElevenLabs remains the primary TTS provider with centralized Sarah voice
  configuration.
- Cloud synthesis failure now falls back automatically to the local Zi-Nanami
  command provider without exposing credentials or changing provider priority.
- A regression test covers cloud failure, local fallback output, and call order.
- Current suite result after the fallback change: 27 passed, 0 failed.

## STT isolation continuation — 2026-08-28

- Continued the existing native-crash mitigation without rebuilding the voice
  subsystem. The authenticated STT endpoint remains isolated through
  `app.voice.stt_worker`.
- Worker launch failures now return `STT_WORKER_UNAVAILABLE`; malformed successful
  worker output returns `STT_WORKER_INVALID_OUTPUT`; timeout and crash handling remain
  fail-closed.
- The worker protocol now preserves handled transcription failures, rejects provider
  impersonation and malformed/oversized results, and terminates the child on timeout
  or request cancellation. Transcript events use only the validated provider field.
- Hermes ordinary requests now use the configured LLM primary; prompt heuristics no
  longer alter provider priority. Explicit `/model` remains user-directed. The frozen
  HostRuntime modality router still owns bounded fallback.
- The current Python App is validated as a simulator/operator prototype only. It has
  no HostRuntime real-target adapter and cannot establish physical motion authority or
  provide commissioning evidence.
- Full application suite passed: 38 tests. The fallback runner passed 28 tests;
  compileall and `git diff --check` passed.

## App command-center software completion — 2026-08-28

- Added a narrow semantic `HostRuntimeAdapter` contract with closed simulation,
  unavailable-real, and reserved future-real target modes. Requests and responses
  distinguish requested/accepted/rejected/confirmed, delivery, physical confirmation,
  authority generation, and session identity.
- The App rechecks adapter mode/generation/session across commands and rejects stale
  authority before publishing execution. Configuration cannot create real authority;
  restart begins `DISCONNECTED` without a real adapter.
- Added honest authenticated status surfaces for runtime/readiness, extensions,
  independent LLM/STT/TTS/Wake provider chains, voice, hardware profile,
  configuration, presentation, and diagnostics. Unimplemented operations report
  `not_exposed`, `not_implemented`, or `unavailable`; no mutation/raw-control endpoint
  was added.
- Provider status is ordered and capability-specific but reports authorization as
  `not_verified`; the App does not impersonate frozen HostRuntime authority.
- Independent adversarial re-review found no remaining concrete raw-control,
  fabricated physical state, duplicate authority, stale revival, provider widening,
  secret leakage, simulator/real confusion, optional-service safety masking,
  documentation overclaim, or commissioning-claim issue. Production adapter
  authentication/transport/live authorization remains explicitly deferred.
- Validation: full App pytest passed 63 tests; fallback regression runner passed 35;
  Python compileall, PowerShell parsing, and `git diff --check` passed.

## Final application integration checkpoint — 2026-08-28

- Authenticated `/api/plugins` reports secret-free plugin health and permissions;
  the HUD consumes those statuses instead of treating every panel as decorative.
- Robot command, state, telemetry, and E-stop events are published through the
  bounded event bus and consumed by the HUD reducer. Simulator telemetry is labeled
  `SIMULATION`.
- The HUD provides real chat, voice playback, stop-speech, simulator state controls,
  E-stop, health refresh, and live WebSocket event display.
- Health reporting includes Zi-E, Hermes session/model, voice, browser backend,
  simulator/robot, and real local system metrics without secrets.
- Full relevant application suite: 27 passed, 0 failed.
- Live checks passed for server lifecycle, health, HUD delivery, plugin registry,
  Hermes chat, ElevenLabs audio, simulator command, E-stop, and restart recovery.
# 2026-08-30 target architecture and XiaoZhi donor preparation

Current working baseline is pushed `d8f6e1c12de60d78c3a3ee444111a2d586d79455`
with this session uncommitted. Target Architecture Freeze Candidate V1 now fixes the
responsibility chain HostRuntime -> authenticated semantic ESP32 Presence Runtime ->
semantic STM32 Safety/Motion Authority without freezing physical bindings.

XiaoZhi ESP32 was audited as an MIT engineering donor at pinned commit
`374a5ccf95c2ed513dbad9ca204adbac75062dce`. No donor source/assets were copied and no
runtime dependency was added. Official Espressif/LVGL components are preferred for
speech, camera, audio codec/device/effects, display port, and LEDs; actual versions
remain target/build decisions after HW-002 verification.

Host-testable Zi-E-owned Face Engine/Face Pack validation and fixed local safety-voice
grammar now exist. They are data/observation logic only and grant no robot authority.
Host/ESP32 and ESP32/STM32 documents are semantic boundaries, not live transports.
ESP-IDF and STM32 vendor toolchains remain absent on this machine, production targets
remain disabled, HW-002 remains ordered/unverified, first power is not authorized,
physical commissioning is not done, and Phase2B2 remains
`WAITING_FOR_VERIFIED_INPUTS`.
