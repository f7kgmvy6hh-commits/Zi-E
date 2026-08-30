# ZI-E Control Center 0.05

Status: evidence-driven maintenance/hardening cockpit; physical
HostRuntime adapter and robot remain deliberately unimplemented/uncommissioned.

Version history: `0.01` is the structural Project Cockpit foundation. `0.02` adds
functional simulation/preview workspaces and fixed local developer launch workflows.
`1.00` remains reserved for a future genuinely integrated release-capable system.
`0.03` adds contained purchased-inventory persistence and engineering reconciliation.
`0.04` adds required-hardware coverage and practical evidence/readiness reports.
`0.05` adds schema-validated project/bench evidence, truthful software-versus-
physical readiness, bounded repository-metadata caching, non-overlapping HUD refresh,
memory-only browser tokens, and CSV export formula neutralization. App versions track
software changes; physical evidence entry alone does not force a bump.

## Presence cockpit implementation

The HUD derives version 0.05 from the backend and presents a truthful Host/ESP32/STM32
graph, prioritized alerts, top safety/controller truth, procedural semantic Face
preview, Face Pack metadata, provider flow, development readiness, and bounded hardware
evidence. HW-002 is explicitly NOT_ARRIVED/UNVERIFIED; spare-board evidence cannot
promote it. Existing inventory, commissioning, simulation and developer workflows
remain intact.

One scheduler owns the enhanced refresh path. A new generation aborts the previous
request group, stale generations cannot render, hidden tabs skip noncritical polling,
and visibility restoration refreshes immediately. The event stream reconnects once
and retains at most 100 visible entries. Repository metadata alone uses a two-second
in-process cache; live runtime/safety state is never cached by that layer.

## Purchased hardware workflow

The active store is fixed at project-owned `runtime/inventory/` and ignored by Git.
It starts empty and never imports design candidates. The exact existing 32-column CSV
is unchanged; 0.03 metadata (physical status, storage location, evidence/review state,
driver state, candidate match, commissioning dependencies, revision timestamps,
tombstones and history) is stored separately in the JSON state.

Normal intake fields are deliberately small. Engineering review is explicit and
separate. Every mutation includes the expected revision; stale writes return conflict.
Validated writes use a same-directory temporary file and atomic replacement, while a
validated previous state is retained for fixed rollback. Import requires preview then
explicit commit. Export is a deterministic browser download in canonical order and
neutralizes formula-leading cells without mutating stored inventory.
Rollback is exposed by the narrow authenticated backend and audited; a dedicated HUD
rollback button is deferred until a revision-comparison confirmation view is added.

`VERIFIED` requires: received physical status, exact manufacturer and model, reviewed
evidence state, reviewed review state, evidence source, reviewer, review date, and an
explicit verification request. Receipt, model text, purchase/photo links, CSV import,
candidate matching, tests, and simulation cannot satisfy this transition.

Google Sheet workflow: use the ten simple intake columns documented in
`PURCHASED_PARTS_INTAKE_SCHEMA.md`, export CSV, map into the canonical template,
preview in Control Center, validate, explicitly persist, then complete engineering
review and reconciliation. No Google credentials or API integration exists.

The reconciliation surface also derives `first_bench_readiness`: a read-only,
no-authority evidence-closure report that feeds exact item requests into the existing
WHAT I NEED FROM YOU queue. It distinguishes unpowered work from conditional future
isolated tests and never authorizes first power or commissioning.

The Control Center serves five compatible roles without acquiring hardware authority:
development cockpit, hardware-intake tool, commissioning console, simulator UI, and
eventual semantic real-robot operator interface.

## Workspaces and honesty model

The desktop-first HUD contains Overview, Robot, Camera/Vision, Face/Display/RGB,
Motors/Actuators, Sensors, Power/Battery, Controllers, Voice/Providers, Hardware
Inventory, P1/CAD, Commissioning, Safety, Plugins/Profiles, Diagnostics, and Developer.
Unavailable fields remain explicit `UNAVAILABLE`, `NOT_CONFIGURED`, `NOT_VERIFIED`, or
`VERIFY_ON_ARRIVAL`; null values are never replaced by demonstration telemetry.

Robot state buttons retain the simulator semantic path. Bounded directional drive
uses hold-to-request UI release stops plus a 500 ms server-side dead-man expiry;
one-at-a-time actuator jog and presentation semantics are functional only
as process-local `SIMULATED`/`PREVIEW` requests. A generated static 640×360 camera
test frame is labeled `TEST_SOURCE` and is not a stream or physical camera. Inventory
import preview is UTF-8, header-exact, in-memory, bounded to 2 MiB/500 rows, and
non-persistent until explicit confirmation. Confirmed imports persist through the
contained revisioned store and begin `NOT_VERIFIED`/`REVIEW_REQUIRED`. Commissioning begins
at zero and has no App mutation endpoint. VirtualRobot cannot advance a physical gate.
Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`, with no link transmit operation.

## Developer Workspace

Repository status uses fixed read-only Git invocations. Executable actions are a
closed server-owned allowlist: App pytest, fallback tests, Python compileall, and CAD
validation only when CadQuery is installed. Actions take no arguments, use fixed
working directories, have timeouts and bounded output, require authentication, and
are audit/event logged with `robot_authority: NONE`.

Folder, Windows Terminal, Codex, and log launch entries use fixed server-owned command
tuples, repository-root discovery, no browser arguments, and detached external local
applications. Codex uses `--cd <root> --sandbox workspace-write --ask-for-approval
never --search`; `--yolo` is forbidden. Logs must resolve under the repository
`runtime/` directory. Hermes launches only when the existing configuration is exactly
`hermes` and the executable/terminal are discoverable; any parameterized command keeps
it `UNAVAILABLE`. There is no generic shell, arbitrary path/filesystem, CAN transmit,
raw actuator, or unrestricted flashing endpoint. All developer actions have robot
authority `NONE`.

## Boundary

The command center is a loopback-only operator surface intended to consume ZI-E's
public semantic robot boundary. Hermes remains the agent brain and owns provider/OAuth
secrets, session memory, and tools. The command center owns authentication, display
state, redacted audit logs, local metrics, event delivery, voice routing, and
high-level robot state requests. It has no raw actuator, generic terminal, arbitrary
browser, or unrestricted filesystem API.

The Python state machine is an additional host-side fail-closed guard, not physical
authority or execution confirmation. Firmware/STM32 commissioning, link,
lease/interlock, safe-stop actuation, and actual-state confirmation remain mandatory.
Real mode publishes no telemetry until a future commissioned adapter supplies it.
The present Python implementation is not a HostRuntime adapter and therefore cannot
connect to or authorize a real robot. Until that binding exists, only simulator use is
validated; App state must not be interpreted as execution or commissioning evidence.
With `ZIE_SIMULATOR=false`, ordinary robot commands fail with
`rejected_real_target_unavailable` and the state remains `DISCONNECTED`. The E-stop
endpoint remains available as an operator request surface but returns
HTTP 503 with `requested_not_delivered`; it does not claim a physical stop. Health, plugin, state,
and event surfaces label this condition `real-target-unavailable`/`not_delivered` and
never infer a physical source from configuration alone.

## Run

```powershell
.\scripts\setup.ps1
.\scripts\start.ps1
.\scripts\health.ps1
.\scripts\stop.ps1
```

Open `http://127.0.0.1:8766/`, then enter the generated `.env` bearer token in the
top-bar token field. Start refuses a live ZI-E PID; stop verifies that the recorded PID's
command line is the ZI-E server before terminating it.

## Public surface

- `GET /api/health`, `/api/state`, `/api/settings`, `/api/runtime`
- `GET /api/plugins`, `/api/extensions`, `/api/providers`, `/api/voice/status`
- `GET /api/hardware-profile`, `/api/configuration`, `/api/presentation`,
  `/api/diagnostics` (honest read-only visibility; no authority mutation)
- `GET /api/control-center`, `/api/developer`, `/api/semantic-contracts`
- `GET /api/hardware/inventory/schema`; `POST /api/hardware/inventory/preview`
- `POST /api/developer/action` with a fixed allowlisted `action_id` only
- `POST /api/chat`
- `POST /api/robot/command`, `POST /api/robot/estop`
- `POST /api/voice/mute`, `POST /api/voice/stop`
- `WS /api/events`; first JSON message: `authorization: Bearer ...`

All REST routes require bearer authentication. Primary tests are pytest tests under
`tests/app/`. When pytest cannot be installed offline,
`python scripts/run_tests_fallback.py` executes the current plain-assert tests with
their two simple fixtures. It is explicitly a fallback, not a pytest replacement.

`app.runtime.adapter.HostRuntimeAdapter` is the only future real-target integration
surface. See `APP_HOSTRUNTIME_ADAPTER_CONTRACT.md`. The current simulator and
unavailable adapters contain no raw-control methods; profile/configuration/pack and
provider changes remain future capability-scoped HostRuntime operations.
# Target-architecture status continuity

Future provider, ESP32/STM32/controller-link, and Face Engine panels remain derived
status/configuration under existing Control Center authority. They may show independent
provider modality health, Face mode/pack/state, Internet/Hermes/device/link state, and
copied motion-authority state. They must not add raw GPIO/PWM/CAN/actuator controls or
infer authority from connectivity. No Control Center redesign was made in this pass.
