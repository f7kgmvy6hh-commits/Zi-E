# Repository map

The root [README](README.md) is the human entrypoint. This file maps active and
historical areas; [PROJECT_CONTEXT](PROJECT_CONTEXT.md) retains stable product
context and [AGENTS.md](AGENTS.md) defines collaboration/safety rules.

## Active implementation

- app/ — HostRuntime-facing Control Center, bounded runtime/provider models,
  inventory/reconciliation, security, and HUD assets.
- firmware/ — vendor-neutral contracts, host tests, ESP32/STM32 target boundaries,
  profiles, and driver ownership.
- scripts/ — fixed setup, validation, toolchain, host-build, and no-flash generic
  ESP32 workflows.
- tests/ — App, scripts, simulation, and repository contract tests.
- software/ and simulation/ — higher-level behavior and simulation areas.
- electronics/ — current electrical design/evidence area; unverified claims remain
  gated.
- mechanical/cad/current/ — current mechanical source, generated deliverables, and
  validation data.
- mechanical/cad/archive/ — superseded CAD snapshots.
- assets/concepts/ — visual direction references only.
- assets/deprecated/ — known-invalid generated visuals.

## Active documentation

- docs/CURRENT_STATE.md — single concise current-status authority.
- docs/ZI-E_MASTER_SPEC.md — authoritative system specification.
- docs/MODULAR_HARDWARE_ARCHITECTURE.md — ownership and safety boundaries.
- docs/DESIGN_DECISIONS.md — append/supersede decision log, not a status dashboard.
- docs/OPEN_ITEMS_AND_FREEZE_GATES.md — unresolved physical validation gates.
- docs/ENGINEERING_METHOD_AND_MEMORY_LOOP.md — mandatory engineering/review method.
- docs/COMPONENT_MASTER_MATRIX.md — design candidates/backups/switch conditions.
- docs/evidence/ZI-E_PROJECT_EVIDENCE.json — bounded, validated Control Center
  software/bench evidence; never a physical-authority source.
- docs/conversations/ — durable session/bench summaries.
- docs/archive/status/ — superseded status narratives.
- docs/archive/procurement/ — point-in-time procurement conversions, not current
  physical verification.
- docs/REPOSITORY_ORGANIZATION.md — active/archive organization policy.

## Historical/non-authoritative

- legacy/exports/ZI-E_lite_v0.1/ — historical lite export formerly at root.
- legacy/exports/zi-e-v0.3-2026-08-23/ — v0.3 all-in-one migration export and its
  historical file/checksum manifests.
- legacy/snapshots/git-bundles/ — preserved Git bundle snapshots formerly split
  across root and repo_history/.
- Other legacy/ content — untrusted reference input.

Historical relocation preserves content and does not reduce Git history size. Nothing
under legacy/ may override current hardware, pin, power, safety, or commissioning
evidence without explicit review and promotion.
