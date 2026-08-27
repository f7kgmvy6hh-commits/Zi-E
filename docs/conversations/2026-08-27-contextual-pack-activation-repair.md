# 2026-08-27 — Contextual pack activation repair

## Request

Resume from `a5ef482` and make one narrow correction: stop representing contextual
face/sound pack activation with one global `PackState::active`. Support selection in
multiple contexts, isolate replacement, invalidate all affected selections after
registry revocation, make missing-context reads non-creating, preserve validation
state, and require explicit activation after lifecycle recovery. Keep Phase 2B2
`WAITING_FOR_VERIFIED_INPUTS` and add no adjacent architecture or hardware work.

## Outcome

- Pack records now retain only declared/validated data state; context records are the
  sole activation truth.
- Activation creates contexts explicitly, while active-pack queries only search
  existing contexts and fail closed when absent.
- Use-time lifecycle/capability failure clears the revoked pack from every existing
  context, without disturbing other packs or contexts.
- Focused tests cover shared face/sound selection, isolated replacement, all-context
  revocation, read-only missing lookup, preserved validation, and explicit recovery
  activation.
- Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`; no CAN/framing work was added.
