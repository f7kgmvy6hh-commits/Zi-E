# Host Runtime Orchestration and Recovery Foundation

Status: deterministic in-memory GNU C++17 host coordinator and VirtualRobot tests.
This stage coordinates existing authorities; it does not replace or bypass them.

## Runtime lifecycle and ordering

`HostRuntime` is an internal host component with the closed lifecycle `created ->
initializing -> ready -> running/degraded -> stopping -> stopped`, with fail-closed
transition to `failed`. Startup first establishes copied semantic motion state as
`stopped`, explicitly resolves and activates the selected hardware profile, verifies
fresh protected-safety membership, then coordinates extension activation and checks
provider, configuration, presentation, event/state, and VirtualRobot readiness.

Required prerequisite failure dominates optional degradation. A protected-safety,
profile, state-store, or VirtualRobot loss cannot produce degraded-running. Optional
extension/provider/configuration/presentation/event failures are isolated and retained
as typed readiness reasons. Requiredness comes from the authoritative startup plan or
fixed core-domain policy, never from a plugin failure report.

Shutdown and required-failure containment revoke hosted extension epochs in reverse
startup order before deactivating the active profile and preserving stopped semantic
motion state. Each underlying subsystem still performs its own lifecycle,
capability, identity, generation, and context checks.

Health refresh compares the exact runtime-selected profile resolution generation and
every runtime-issued extension epoch/grant set, and rechecks required provider and
configuration authorization. An external same-profile re-resolution, extension
reactivation, or required-service revocation therefore fails closed instead of being
accepted as equivalent authority.

## Bounded readiness

Readiness is returned by value as an immutable snapshot. Each bounded entry contains a
closed subsystem domain, stable semantic name, ready/degraded/unavailable/failed state,
typed reason, monotonic readiness generation, and required/optional policy. Updating a
subsystem replaces its current entry; the runtime does not retain unbounded diagnostic
history. Unknown domains and reasons fail closed.

## Safe checkpoint and recovery contract

The `1.0.0` checkpoint is copied data only. It may preserve runtime generation,
selected profile ID/revision, exact selected package/logical-device/capability tuples,
active configuration generations, expected extension lifecycle intent, provider
semantic preference, and presentation context/pack selections.

It never contains command sessions, SDK handles, motion leases, setpoints, raw
hardware state, controller heartbeat authority, safety bypasses, or replayable command
sequence authority. Checkpoints are not authority.

Recovery is explicitly split:

1. `prepare_recovery` validates the exact checkpoint schema and bounds, establishes
   stopped semantic state, explicitly re-resolves the profile, and compares the full
   selected-device set against the checkpoint. A spare or replacement device is not
   silently accepted.
2. Configuration generations and presentation selections are revalidated against
   their existing authorities.
3. `complete_recovery` explicitly activates the resolved profile and reauthorizes
   extensions, producing fresh registry generations, SDK epochs, and command sessions.

Recovery attempts are bounded. Corrupt/unknown versions, missing required state,
selection changes, and repeated failures remain typed failures.

## Integration and exclusions

The coordinator uses `HardwareProfileManager`, `ExtensionHost`, `ProviderRouter`,
`ResilientEventBus`, `RobotStateStore`, `PackCatalog`,
`TransactionalConfiguration`, and `VirtualRobot`. `AuthoritativeRobotCore` remains an
internal dependency used only for semantic stopped-state publication. No raw actuator,
GPIO, PWM, register, safety-bypass, CAN, or driver path was added.

Excluded: persistence/filesystem storage, UI/app, network/cloud, real providers,
dynamic loading, installers, cryptography, transport/framing, hardware drivers,
schedulers/threads, autonomous motion, and commissioning. Phase 2B2 remains
`WAITING_FOR_VERIFIED_INPUTS`.
