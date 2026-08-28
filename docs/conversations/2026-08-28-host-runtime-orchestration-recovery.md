# 2026-08-28 — Host runtime orchestration and recovery foundation

The Product Owner requested one accelerated coherent stage from `41af513`: build the
authoritative in-memory host runtime over the existing profile, registry, Plugin SDK,
provider, event/state, presentation, configuration, and VirtualRobot foundations.

Implemented direction:

- A closed deterministic runtime lifecycle coordinates fail-safe stopped startup,
  explicit profile resolution/activation, protected-safety readiness, hosted extension
  activation, and provider/configuration/presentation/core-service readiness.
- Bounded copied readiness snapshots preserve typed current reasons and generations;
  required failure dominates optional degradation. Plugins cannot declare themselves
  trusted/ready or choose whether their failure is required.
- Shutdown and required failure revoke extension epochs before profile/runtime state.
  Retained SDK handles remain stale.
- Versioned copied checkpoints contain only semantic reconstruction intent. Recovery
  explicitly re-resolves the exact prior device set, revalidates configuration and
  presentation selections, and separately reactivates profile/extensions with fresh
  authority. Replacement devices are never silently substituted.
- VirtualRobot tests cover semantic command/state flow, controlled degradation,
  provider fallback, protected profile loss, shutdown, restart, stale contexts,
  valid and corrupt recovery, no-rebind behavior, retry bounds, and unknown domains.

The open-source harvest reviewed ros2_control lifecycle documentation and issues about
inactive hardware retaining dependent controllers, startup failure containment, and
explicit hardware restart. Zi-E adapts strict prerequisite ordering, reverse authority
revocation, bounded retry, and explicit reactivation. No external code was copied.

Final validation used fresh MSYS2 UCRT64 GNU g++ 16.1.0 C++17 configure/build
trees. Normal and warning-clean (`-Wall -Wextra -Wpedantic -Wswitch-enum`) builds
passed, both CTest runs passed 1/1, and `git diff --check` passed.

Independent adversarial review found and drove repairs for active-profile/device
substitution, required-service refresh, caller-controlled failure criticality,
pre-active and post-start extension epoch/grant replacement, checkpoint aggregate
bounds/configuration intent, deterministic device comparison, and shutdown revocation
reporting. The repaired paths have focused regression coverage.

Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`. The session stops uncommitted at
`READY_TO_COMMIT`; no next stage was started.
