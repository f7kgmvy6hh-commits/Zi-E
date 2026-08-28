# ZI-E Control Center V1

Status: App software foundation complete with semantic adapter contract; physical
HostRuntime adapter and robot remain deliberately unimplemented/uncommissioned.

The Control Center serves five compatible roles without acquiring hardware authority:
development cockpit, hardware-intake tool, commissioning console, simulator UI, and
eventual semantic real-robot operator interface.

## Workspaces and honesty model

The desktop-first HUD contains Overview, Robot, Camera/Vision, Face/Display/RGB,
Motors/Actuators, Sensors, Power/Battery, Controllers, Voice/Providers, Hardware
Inventory, P1/CAD, Commissioning, Safety, Plugins/Profiles, Diagnostics, and Developer.
Unavailable fields remain explicit `UNAVAILABLE`, `NOT_CONFIGURED`, `NOT_VERIFIED`, or
`VERIFY_ON_ARRIVAL`; null values are never replaced by demonstration telemetry.

Robot state buttons retain the simulator semantic path. Direction, speed, actuator
jog, presentation and flashing contracts are visible but `NOT_CONFIGURED`. Inventory
preview is UTF-8, header-exact, in-memory, bounded to 2 MiB/500 rows, non-persistent,
and always marks rows `NOT_VERIFIED`/`REVIEW_REQUIRED`. The commissioning view begins
at zero and has no App mutation endpoint. VirtualRobot cannot advance a physical gate.
Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`, with no link transmit operation.

## Developer Workspace

Repository status uses fixed read-only Git invocations. Executable actions are a
closed server-owned allowlist: App pytest, fallback tests, Python compileall, and CAD
validation only when CadQuery is installed. Actions take no arguments, use fixed
working directories, have timeouts and bounded output, require authentication, and
are audit/event logged with `robot_authority: NONE`.

Folder/Terminal/Codex/Hermes/log launch entries remain `UNAVAILABLE` until fixed local
workflows are reviewed. There is no generic shell, arbitrary path/filesystem, CAN
transmit, raw actuator, or unrestricted flashing endpoint.

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
