# 2026-08-28 — Plugin SDK and extension-host foundation

## Request

Resume from `de4d566` and implement one in-memory stage defining a stable, versioned,
safe host-facing plugin SDK. Preserve registry/identity/configuration/Semantic Robot
API/provider/event/presentation authority, model revocable extension instances, add
deterministic mocks, keep Phase 2B2 waiting, validate normal and strict GNU C++17
builds, and stop at `READY_TO_COMMIT` without committing.

## Outcome

- Added a public DTO/interface-only SDK contract at version `1.0.0` with explicit
  compatible-range negotiation and fail-closed unknown/future domains.
- Added an internal extension host that issues immutable authoritative contexts,
  capability-scoped semantic services, host command sessions/sequences, and fresh
  activation epochs.
- Independent review found and drove fixes for throwing callback rollback,
  registry-authoritative generation binding, event-subscription retirement, lifecycle
  retry consistency, explicit selected-version reporting, and host-destruction
  revocation.
- Initialization grants no services. Suspension, failure, quarantine, and removal
  immediately retire handles; recovery is explicit and issues a new context only after
  a separate activation.
- Asset packs and protected/embedded modules receive no executable host context. Wake
  and provider categories cannot obtain motion; no raw-control/protected-safety SDK
  type exists.
- Configuration access reuses typed transactional staging with host-supplied binding;
  extensions cannot validate/commit authority escalation.
- Deterministic tests cover the requested compatibility, authority, lifecycle,
  stale-handle, service-scope, configuration, command, event, provider, presentation,
  recovery, re-registration, and duplicate-instance cases.
- Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`; no loader, dynamic linking,
  sandbox, persistence, network, transport, or hardware work was added.
