# 2026-08-27 — Provider routing repair and presentation packs

## Request

Resume from `1d7db12`, first preserve bounded typed provider failure information and
route by an explicit requested semantic capability, then add a data-only presentation
engine with face/sound packs, deterministic selection/replacement, registry revocation,
authoritative state, virtual integration, and adversarial tests. Exclude all real
providers, media loading/decoding, display/audio hardware, persistence, network,
dynamic loading, and CAN/framing. Keep Phase 2B2 `WAITING_FOR_VERIFIED_INPUTS`, validate
fresh C++17 and warning builds, and stop uncommitted at `READY_TO_COMMIT`.

## Outcome

- Provider outcomes retain bounded typed diagnostics across successful fallback and
  exhaustion. Shared semantic routing capability is distinct from unique registry
  authorization capability; wrong-capability providers are never called.
- Data-only face/sound contracts validate identity, versions, schema/domain, names,
  references, defaults/fallbacks, cue kind, and abstract asset handles against registry
  records that have no entrypoint or permissions.
- The bounded catalog provides declared/validated/active state, one selection per pack
  type and context, deterministic replacement, and use-time lifecycle/capability checks.
- The engine and virtual robot process only accepted semantic presentation/audio
  tokens and mutate state only through authoritative core access.
- Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`; no wire implementation was added.
