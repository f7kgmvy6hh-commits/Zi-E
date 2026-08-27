# 2026-08-27 — Event delivery and provider foundation

## User request

Resume from `2b2b176` as one coherent stage: repair `ResilientEventBus` subscriber
failure semantics with bounded retry and deterministic dead-letter accounting, and add
vendor-neutral, swappable, registry-authorized, capability-checked, bounded-failover
LLM/STT/TTS/wake provider foundations with deterministic mocks and adversarial tests.
Preserve existing safety/trust/identity boundaries, keep Phase 2B2
`WAITING_FOR_VERIFIED_INPUTS`, exclude real integrations and hardware-adjacent work,
run fresh validation, update the checkpoint, and stop at `READY_TO_COMMIT` without
committing or pushing.

## Implemented outcome

- Subscriber callback failures retry the same queued event up to an explicit bound,
  then remove it and increment one saturating per-subscriber dead-letter count.
- A provider router accepts only exact active registry package/device/category and
  unique kind-prefixed capability bindings, rechecks them on every invocation, and
  applies deterministic ordered failover under provider-count and attempt bounds.
- Vendor-neutral semantic contracts and deterministic mocks cover LLM, STT, TTS, and
  wake without networking, secrets, audio paths, persistence, scheduling, or hardware.
- Tests include transient recovery, dead-letter exhaustion, provider exception and
  malformed-response isolation, bounded failover, impersonation, capability mismatch,
  invalid input, and lifecycle revocation.
- Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`; no wire/framing work was added.
