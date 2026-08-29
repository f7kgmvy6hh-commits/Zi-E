# 2026-08-29 — Control Center 0.02 functional workspaces

The Product Owner established deliberately slow project/App versioning: `0.01` is the
existing Control Center foundation at `1c1cf43`, `0.02` is the functional-workspace and
safe-developer-tool stage, and `1.00` is reserved for a much later integrated release.

The session continued from the authoritative clean repository HEAD without reset,
stash, commit, or push. It added one authoritative App version source; enriched the
daily overview and honest camera, presentation, actuator, sensor, power, controller,
link, inventory, commissioning, and safety surfaces; added process-local semantic
presentation, drive, and single-active-actuator simulation interactions; and added a
generated static camera test source.

Developer capabilities remain separately allowlisted and argument-free. Repository,
terminal, Codex, Hermes (only with exact fixed configuration), and contained log
launches use server-owned paths/arguments and have robot authority `NONE`. No physical
driver, production CAN, raw control, generic shell, arbitrary filesystem access,
unrestricted flashing, or physical commissioning claim was introduced. Phase 2B2
remains `WAITING_FOR_VERIFIED_INPUTS` and autonomous physical motion remains blocked.

Deferred for 0.03+: inventory/evidence persistence, richer authoritative simulator
sources, approved non-default Hermes launch configuration if required, physical
HostRuntime integration, and all verified-input-dependent hardware work.

Fresh validation passed 76 App pytest tests and 44 fallback tests plus compileall,
PowerShell, HTML, JavaScript, and diff checks. Independent adversarial review found
four issues; all were repaired with server dead-man expiry/client release stop, visible
physical blockers in simulation, honest detached-launch acceptance, and mismatched
actuator-stop rejection.
