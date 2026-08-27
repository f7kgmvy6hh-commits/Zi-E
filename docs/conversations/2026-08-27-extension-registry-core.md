# 2026-08-27 — Extension registry core

The user requested one accelerated coherent in-memory stage combining extension
registration, lifecycle, and capability activation/resolution from clean commit
`a323e3a`. Phase 2B2 remained `WAITING_FOR_VERIFIED_INPUTS`; no CAN/framing work or
values were introduced.

The implementation binds candidate manifests and device identities to external registry
assignment of package ID, trust, controller, and profile; validates before acceptance;
rejects duplicates/conflicts; and retains removal tombstones. It extends the existing
lifecycle vocabulary with validated/inactive/activating and enforces explicit legal
edges. Capability resolution is package-and-device bound, exposes active/degraded
providers only, and rejects ambiguous activation. Quarantine, disable, failure,
inactivity, and removal synchronously revoke active capabilities.

Adversarial host tests cover trust/identity/ownership escalation, lifecycle misuse,
unvalidated activation, ambiguity, unknown domains, and revocation. No persistence,
loader, parser, package acquisition, sandbox, signatures, UI, discovery, or hardware
implementation was added.
