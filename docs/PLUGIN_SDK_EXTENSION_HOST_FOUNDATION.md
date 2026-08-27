# Plugin SDK and Stable Extension Host API Foundation

Status: in-memory GNU C++17 contract and deterministic host tests. The only public
plugin compilation boundary added by this stage is
`firmware/include/zie/sdk/PluginSdk.hpp`. `firmware/core/ExtensionHost.hpp` and
`AuthoritativeRobotCore.hpp` are internal host implementation headers and are not SDK
surface.

## Contract compatibility

The SDK contract is semantic version `1.0.0`. An extension declares an inclusive
minimum/maximum contract range. The host accepts only a well-formed range that
intersects the explicit host-supported range (`1.0.0` through `1.0.0`). Zero/reversed
ranges, another major, and a minimum newer than the host range fail closed. A plugin
range may have a future maximum when it also includes `1.0.0`; the issued context
explicitly reports that the selected contract is still `1.0.0`. The host never
executes or guesses compatibility for an unknown future contract or enum domain.

The manifest schema version, manifest plugin API version, extension package version,
and this SDK contract remain distinct version axes. Passing one does not imply that
another is compatible.

## Host-issued context

`sdk::ExtensionContext` is an abstract read-only interface implemented only by the
internal host. It exposes registry-authoritative package ID, logical device ID,
hardware-profile binding, assigned trust, lifecycle-at-issuance, instance epoch,
validated capabilities, active capabilities, and capability-scoped service handles.
There are no identity/trust/capability setters and no concrete SDK context class for
extension code to instantiate.

Initialization receives an initialized context with validated metadata, no active
capabilities, and no service handles. Activation is a separate authoritative registry
transition and issues a new active context. Every activation gets a fresh nonzero
epoch backed by the registry's monotonic authorization generation; the command session
is bound through `AuthoritativeRobotCore`, not selected by the extension.

## Safe services

The SDK contains only narrow semantic interfaces:

- semantic command submission without caller-supplied package, device, session, or
  sequence identity;
- typed event subscribe/delivery with host-namespaced subscriber identity;
- copied read-only robot-state snapshots;
- copied active configuration and typed staging requests without caller-supplied
  authoritative binding;
- typed LLM/STT/TTS/wake provider invocation where the caller has the corresponding
  `provider.invoke.*` capability;
- copied semantic presentation state.

Adapters translate SDK DTOs into the existing Semantic Robot API, resilient event bus,
robot-state store, transactional configuration, provider router, and presentation
state. Each call first checks the current epoch, exact registry package/device,
`active` lifecycle, and fixed required active capability. The existing subsystem then
performs its own checks. The adapter is therefore not a privileged shortcut.

There is no SDK type for GPIO, PWM, registers, raw motor/actuator commands, CAN frames,
protected safety operations, authority mutation, or direct state mutation.

## Capability policy

Only executable `host_plugin` records receive an SDK context. Data-only asset packs,
embedded modules, and protected safety modules do not enter this host execution path.
Known capabilities are mapped to service/domain policy; unknown strings fail
validation. Category policy additionally blocks motion for provider/wake categories.
A wake provider may publish a registry-authorized wake capability but receives no
motion or provider-consumer handle. A presentation-only extension may receive the
semantic command service, but motion submission is denied by the fixed live capability
check.

## Lifecycle and stale-handle defense

The host models declared, validated, initialized, active, inactive, failed,
quarantined, and removed states over the authoritative registry lifecycle.
Initialization grants no active capability. Suspension, failure, quarantine, and
removal invalidate the current epoch before completing the registry transition, so
retained handles immediately fail closed. Registry-side revocation also changes the
authorization generation, so a lifecycle change outside one host adapter cannot revive
its retained handles. Recovery is explicit and only returns the record to inactive; a
separate activation issues a new epoch/context. Throwing initialization/activation or
cleanup callbacks are contained; activation exceptions revoke the issued authority and
move the registry record to failed.

Host destruction retires every still-issued authority and unsubscribes its tracked
event subscriptions before releasing internal state. Retained service handles then
return stale-context failures without dereferencing backend services.

Removal leaves a retired host instance and invalid handles. The registry may accept an
explicit, fully revalidated replacement registration for that removed package, but the
replacement must receive a new registry-generated logical instance identity. The new
host instance also receives a different authorization generation and command session.
Reusing the same extension object as another host instance is rejected. A retired
object/context is never reactivated.

## Configuration boundary

The SDK can read a copied active snapshot and request staging of typed semantic values.
It cannot validate or commit its own candidate. The host supplies package/device/profile
binding to `TransactionalConfiguration`; the existing declaration, revision,
validation, lifecycle, and commit rules remain authoritative. Trust, permission,
capability, package, device, identity, controller, profile, protected-safety, GPIO,
PWM, register, actuator, driver, and CAN namespaces cannot become active
configuration.

## Validation coverage and exclusions

Deterministic mocks cover compatibility, incompatible/future versions, unknown
domains/capabilities, authoritative context data, initialization without authority,
service visibility, backend authorization, event delivery, configuration escalation,
immediate revocation, explicit recovery, quarantine, removal/re-registration epochs,
asset packs, wake providers, and duplicate instances.

This stage deliberately adds no loader, dynamic linking, WASM/process sandbox,
filesystem discovery, manifest parser, installer, signing, persistence, network,
secrets, UI, scheduling, transport, CAN/framing, or hardware driver. In-process C++
plugins are not a security sandbox; future runtime isolation remains a separate gate.
Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`.
