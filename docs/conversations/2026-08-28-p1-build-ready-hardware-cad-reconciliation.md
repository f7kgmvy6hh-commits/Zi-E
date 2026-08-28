# Prototype P1 build-ready and hardware/CAD reconciliation — 2026-08-28

## Scope

Continued from clean authoritative HEAD `1ef73b7`. Audited current hardware/CAD/BOM,
P1, risk, roadmap, design and mechanical sources without restoring archived choices or
inventing purchased parts/measurements.

## Artifacts

- Canonical hardware/module reconciliation and conflict register.
- Empty Google Sheet/CSV physical inventory intake contract.
- P1 CAD fixed/parametric/measurement-blocked/optional freeze candidate.
- Assembly, fastener, service, wiring, power and first-power package.
- Arrival-day physical verification worksheet and P1 no-go sign-off.
- Condensed Phase 2B2 Classical CAN input gate.
- Updated P1 package, component/CAD pointers, decisions, risks, current state and
  roadmap.

## Safety disposition

No hardware is claimed physically verified. v0.3 CAD numerical results remain model
outputs. Physical commissioning is not done; production drivers/CAN are not started;
autonomous motion remains prohibited. Phase 2B2 remains
`WAITING_FOR_VERIFIED_INPUTS`.

## Validation

- Committed CAD dataset/schema: 39 checks, 38 pass, one restricted.
- Purchased-parts template: header-only, 32 unique required fields.
- Fresh CAD regeneration could not run because CadQuery is not installed. The release
  checksum manifest also disagrees with an untouched STEP file in this checkout, so
  neither regeneration nor full artifact-integrity validation is claimed.
- Independent adversarial review found omitted named alternatives and incomplete BOM
  status representation; both were repaired. It otherwise found no invented facts,
  unsafe authority mapping, premature CAN freeze, or commissioning overclaim.
- Final `git diff --check` is required before handoff. No commit or push was performed.
