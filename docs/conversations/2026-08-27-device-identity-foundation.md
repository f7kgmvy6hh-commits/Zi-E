# 2026-08-27 — Device identity foundation

The user requested one independent, dependency-safe foundation from committed baseline
`08e4f93`: a stable typed device identity model. Phase 2B2 had to remain
`WAITING_FOR_VERIFIED_INPUTS`, with no invented CAN values and no loader, persistence,
discovery, parser, database, cloud, or UI work.

Implemented an in-memory C++17 model separating extension package, physical hardware,
logical instance, controller, and hardware-profile binding. Added explicit provenance
and trust, provisional-local identity for hardware without trustworthy serials,
fail-closed collision/domain validation, registry-only controller/profile assignment,
protected-controller trust checks, and identity immutability after activation.

Independent review found that candidate-carried assignment authority could be confused
for registry proof and that provisional metadata was omitted from the activation
immutability comparison. The repair removed the authority field, bound package,
controller, and profile to external registry context, and compares the complete
physical identity record after activation. Adversarial regression tests cover both.

Research reviewed Home Assistant registry/collision and real name/discovery-order
failure motivation, ROS 2 immutable loaded-instance identity, and systemd transient to
persistent identity handling. No external code or artifacts were copied.

Focused positive and negative host tests were added. Final validation and worktree
review are recorded in the session handoff; no commit or push was performed.
