# App real-target fail-closed correction — 2026-08-28

## Finding

GitHub review found that `simulator=false` still let the local Python
`RobotController` accept transitions and caused events to label local state as
physical, despite there being no HostRuntime real-target adapter.

## Repair

- Non-simulation ordinary commands return `rejected_real_target_unavailable` and keep
  the controller `DISCONNECTED`.
- Non-simulation E-stop remains an operator request but returns
  `requested_not_delivered`; it does not claim physical acceptance.
- Health, plugin, state, and event data explicitly report
  `real-target-unavailable`, `unavailable`, and `not_delivered`.
- Simulation behavior remains unchanged. No raw hardware path, HostRuntime bypass, or
  fabricated execution confirmation was added.

## Validation

The final App pytest, fallback runner, compileall, PowerShell parsing, and diff-check
results are recorded in `docs/CURRENT_STATE.md`. No commit or push was performed.
