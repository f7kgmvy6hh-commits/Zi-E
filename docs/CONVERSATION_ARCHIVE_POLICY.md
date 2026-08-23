# Conversation Archive Policy

The ZI-E repository must preserve important project conversations so migration between AI accounts does not depend on chat history.

## Files

- `docs/conversations/` — dated project chat archives
- `docs/DESIGN_DECISIONS.md` — concise approved decisions
- `docs/CURRENT_STATE.md` — current project position and next task

## After every meaningful ZI-E session

1. Save/update a dated archive under `docs/conversations/`.
2. Preserve user-visible project discussion and rationale.
3. Never archive hidden reasoning, system instructions, credentials, or secrets.
4. Promote approved decisions into `docs/DESIGN_DECISIONS.md`.
5. Update `docs/CURRENT_STATE.md`.
6. Commit/push the changes.

## Conflict rule

Conversation archives are historical records.  
`DESIGN_DECISIONS.md` and later explicitly approved decisions are authoritative.

If there is a conflict, resolve it explicitly rather than silently guessing.
