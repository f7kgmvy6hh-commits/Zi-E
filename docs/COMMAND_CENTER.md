# ZI-E V1 Local Command Center

Status: App software foundation complete with semantic adapter contract; physical
HostRuntime adapter and robot remain deliberately unimplemented/uncommissioned.

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
Settings panel. Start refuses a live ZI-E PID; stop verifies that the recorded PID's
command line is the ZI-E server before terminating it.

## Public surface

- `GET /api/health`, `/api/state`, `/api/settings`, `/api/runtime`
- `GET /api/plugins`, `/api/extensions`, `/api/providers`, `/api/voice/status`
- `GET /api/hardware-profile`, `/api/configuration`, `/api/presentation`,
  `/api/diagnostics` (honest read-only visibility; no authority mutation)
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
