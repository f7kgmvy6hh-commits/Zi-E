# 2026-08-27 — Transactional configuration foundation

Starting from clean `cd92694`, the user first requested repair of the registry ACTIVE
gate, then one coherent in-memory transactional configuration foundation. Generic
`transition(active)` is now rejected from activating and degraded states; activation
must use capability validation/ambiguity checks.

Configuration now has declared, staged, validated, and active snapshots; monotonic
candidate revision and registry commit generation; authoritative package/logical
device/profile binding; lifecycle commit gates; atomic in-memory snapshot replacement;
and explicit rollback. Failed updates preserve the active snapshot. Trust, permission,
capability, controller/safety, profile-ownership, and raw hardware-control namespaces
are rejected.

No parser, persistence, watcher, UI, cloud sync, loader, CAN/framing, or hardware
application was added. Phase 2B2 remained `WAITING_FOR_VERIFIED_INPUTS`.
