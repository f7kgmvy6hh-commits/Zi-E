# 2026-08-29 — Control Center 0.03 inventory and evidence foundation

The Product Owner requested one coherent software-side purchased-hardware workflow so
the next human activity is data entry rather than more architecture. Work continued
on the complete staged 0.02 tree at unchanged HEAD `1c1cf43`, without reset, stash,
discard, history rewrite, commit, or push.

The exact 32-column canonical CSV was preserved. A separate ignored, project-owned,
revisioned JSON store now holds 0.03 metadata and tombstones. It provides validated
atomic saves, optimistic revisions, one previous revision, fixed rollback, strict
create/edit/review operations, preview/commit import, deterministic export, search,
filters, counters, inspector, evidence/decision controls, reconciliation, blockers,
commissioning dependencies, and Phase 2B2 missing-input visibility.

The store begins empty. Candidate documents never seed purchased inventory. VERIFIED
requires an explicit reviewed-evidence transition and cannot be produced by receipt,
model text, links, photos, import, candidate match, simulator, or tests. No hardware,
HostRuntime, profile, commissioning, developer, or robot authority was added.

Next human workflow: enter actual parts or export the simple Google Sheet to canonical
CSV; preview and persist; attach metadata references; transcribe exact labels; record
documented versus measured evidence separately; reconcile candidates, ownership,
slots/Profile, drivers and CAD; then have an engineer review conflicts and decisions.

Fresh validation passed 104 App pytest tests and 44 fallback tests, compileall,
PowerShell/HTML/JavaScript parsing, exact 32-column schema/migration checks, and diff
checking. Independent adversarial findings were repaired: explicit conflicts and
candidate mismatches block verification, review changes cleanly revoke verification,
unknown CSV controlled values are rejected, revision saves are locked against
concurrent stale writes, and blocker rows navigate to their purchased item.
