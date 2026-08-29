# Control Center 0.04 build-decision session — 2026-08-29

Work continued from pushed base `214ca2d` without reset, commit, or push. The corrected
scope preserved 0.03 persistence and avoided rebuilding generic inventory machinery.
0.04 adds conservative required-hardware, missing-evidence, WHAT I NEED FROM YOU,
CAD/electrical/driver, ESP32/STM32, first-power, commissioning-prerequisite, and Phase
2B2 reports.

Actual inventory remains authoritative only with user evidence; candidates are never
seeded. Reports have robot authority `NONE`, cannot verify hardware or pass physical
commissioning, and Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`. Documentation now
records 0.03 as committed in `214ca2d`, separates App version from physical evidence
progression, and distinguishes the historical Phase A–I plan from the active workflow.

The shutdown-recovery audit preserved the complete working tree and reviewed the diff
for physical-truth, security, and authority regressions. Full App pytest passed exactly
113 tests, and the fallback runner passed exactly 44 tests with 0 failures. Compileall,
PowerShell parsing, HTML parsing, JavaScript syntax, the canonical 32-column inventory
contract, reconciliation invariants, and `git diff --check` passed. Adversarial checks
confirmed that suggestions cannot create purchased coverage, unreviewed text cannot
advance readiness, verification requires explicit reviewed real evidence, simulation
cannot physically confirm, commissioning remains physical-only, real targets fail
closed, and no raw CAN/GPIO/PWM/motor/actuator authority was added.

Actual purchased inventory remains 0. Phase 2B2 remains
`WAITING_FOR_VERIFIED_INPUTS`; physical commissioning remains NOT DONE. The session
stops uncommitted and unpushed at `READY_TO_COMMIT` on unchanged HEAD `214ca2d`.
