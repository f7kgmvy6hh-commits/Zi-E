# 2026-08-31 — Maintenance pass continuation and validation

The maintenance/hardening pass continued from the existing uncommitted tree at
`3243dd78cf5425eb92c0a88cc5dd4b4f58d4bbbe` on branch
`refactor/modular-hardware-architecture`; no reset, checkout, commit, push, flash, or
powered hardware action occurred.

Adversarial review found that the separately bench-tested HW678/S3-N16R8 board had
been assigned to inventory ID HW-001 without an explicit repository link proving the
physical identity. The evidence overlay now names it `SPARE-HW678`; HW-001 remains an
inventory spare awaiting a reviewed photo/order-to-unit link. HW-002 remains ordered,
NOT_ARRIVED, UNVERIFIED, and unflashed. The evidence parser now validates every
hardware item's exact conservative state tuple and fails closed on any promotion.

Additional repairs made the Git safe-directory argument cross-platform, expanded CSV
formula neutralization for whitespace/control prefixes without coercing non-strings,
escaped remaining dynamic HUD values, and invalidated visible readiness evidence on
authentication/request failure. Archive review found that a broad `* -text` rule
would have converted moved checkout line endings into rewritten Git blobs; the rule
was narrowed so all 65 historical move pairs retain their original prospective blobs.

Validation used repository-local pytest temporary storage. The final App suite and
fallback counts, host configure/build/CTest result, frontend/PowerShell parsing,
relative-link audit, secret/truth/artifact scans, and performance measurement are
reported in the Product Owner handoff for this session. The sandbox still cannot
launch EIM (`Access is denied`), so the previous official ESP-IDF v6.0.2 generic
cross-build remains the host-verified result; flash and first integrated power remain
NOT_AUTHORIZED, commissioning remains NOT_STARTED, and Phase2B2 remains
WAITING_FOR_VERIFIED_INPUTS.
