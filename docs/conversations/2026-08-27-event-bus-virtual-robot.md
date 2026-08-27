# 2026-08-27 — Authoritative sessions, resilient events, and virtual robot

The user requested one coherent host-only stage from commit `aee61ba`:

- repair Semantic Robot API replay protection so command sessions are explicitly bound
  and replaced by authoritative core context rather than self-selected by plugins;
- add a deterministic bounded typed event bus with registry/capability checks,
  lifecycle revocation, overflow reporting, and subscriber-failure isolation;
- add a deterministic virtual robot behind the same semantic authorization boundary;
- preserve command acceptance versus execution separation and keep protected/raw
  hardware operations unavailable;
- keep Phase 2B2 `WAITING_FOR_VERIFIED_INPUTS` and make no CAN/framing changes.

The resulting contracts and focused adversarial tests are documented in
`SEMANTIC_ROBOT_API_FOUNDATION.md` and `RESILIENT_EVENT_BUS_VIRTUAL_ROBOT.md`. No
networking, IPC, persistence, scheduler, UI, hardware driver, or autonomous motion was
introduced.
