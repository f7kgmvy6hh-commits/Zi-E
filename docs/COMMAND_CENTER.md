# ZI-E V1 Local Command Center

Status: implemented host-side foundation; physical robot and verified Hermes adapters
remain deliberately unconfigured.

## Boundary

The command center is a loopback-only operator surface around Hermes and ZI-E's public
semantic robot boundary. Hermes remains the agent brain and owns provider/OAuth
secrets, session memory, and tools. The command center owns authentication, display
state, redacted audit logs, local metrics, event delivery, voice routing, and
high-level robot state requests. It has no raw actuator, generic terminal, arbitrary
browser, or unrestricted filesystem API.

The Python state machine is an additional host-side fail-closed guard, not physical
authority or execution confirmation. Firmware/STM32 commissioning, link,
lease/interlock, safe-stop actuation, and actual-state confirmation remain mandatory.
Real mode publishes no telemetry until a future commissioned adapter supplies it.

## Run

```powershell
.\scripts\setup.ps1
.\scripts\start.ps1
.\scripts\health.ps1
.\scripts\stop.ps1
```

Open `http://127.0.0.1:8765/`, then enter the generated `.env` bearer token in the
Settings panel. Start refuses a live ZI-E PID; stop verifies that the recorded PID's
command line is the ZI-E server before terminating it.

## Public surface

- `GET /api/health`, `GET /api/state`, `GET /api/settings`
- `POST /api/chat`
- `POST /api/robot/command`, `POST /api/robot/estop`
- `POST /api/voice/mute`, `POST /api/voice/stop`
- `WS /api/events`; first JSON message: `authorization: Bearer ...`

All REST routes require bearer authentication. Primary tests are pytest tests under
`tests/app/`. When pytest cannot be installed offline,
`python scripts/run_tests_fallback.py` executes the current plain-assert tests with
their two simple fixtures. It is explicitly a fallback, not a pytest replacement.
