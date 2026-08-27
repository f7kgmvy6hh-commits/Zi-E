# Extension Registry Core

Status: in-memory C++17 contract. This stage does not load or execute code and has no
persistence, parser, package acquisition, signatures, sandbox, UI, discovery, driver,
or transport implementation.

## Authoritative registration

`ExtensionRegistry::register_extension` accepts candidate manifest/device data plus a
separate trusted `RegistryAssignment`. The external assignment owns package ID,
`TrustClass`, controller, and hardware-profile binding. Acceptance requires:

- manifest validation under registry-assigned trust;
- exact package binding across assignment, manifest, and device identity;
- device identity validation against the authoritative controller/profile/trust;
- no duplicate live package, physical identity, or logical instance identity.

Candidates have no trust field and cannot overwrite a live package. A removed record
may be replaced only by a new registration with a new logical instance identity that
passes the complete manifest, identity, assignment, and collision checks. A monotonic
registry authorization generation changes on activation and every revocation; host
epochs keep every old context and session retired. Protected STM32 safety modules
require both a built-in trust assignment and matching external
STM32 safety-controller assignment.

## Lifecycle

The registry extends and reuses `LifecycleState`; it does not define a parallel enum.
The managed path is:

```text
installed -> validated -> inactive -> activating -> active
```

Explicit edges support degradation, failure, quarantine, disable, deactivation, and
removal. Unknown states, unknown failure classes, failure metadata on non-failure
states, and unlisted edges fail closed without changing the record. Removed records
remain tombstones until an explicit fully validated replacement registration; they
cannot regain lifecycle or capability state. Quarantine, disable, inactive, failed,
and removal synchronously clear active capabilities.

Generic `transition()` can never enter `active`, including from `activating` or
`degraded`. Initial activation and degraded recovery must pass through
`activate_capabilities()` after the explicit inactive/activating path, so subset and
provider-ambiguity checks cannot be bypassed.

## Capability ownership and resolution

Capabilities follow `declared -> validated -> active`. Existing manifest state
validation enforces subset and duplicate invariants. Activation is accepted only from
`activating`, and a capability already exposed by an active/degraded record causes an
ambiguity rejection. Resolution returns both authoritative package ID and logical
device instance ID and exposes only active/degraded providers.

Permissions remain declarations, not grants. `request_motion` denotes only semantic
Safe Robot API access. No registry API creates a raw actuator, GPIO, PWM, register,
driver, or safety-bypass route.

## Deferred work

Persistence, transactions across process failure, registry migration, dynamic loading,
runtime permission enforcement, sandboxing, signatures/revocation, dependency graphs,
package installation, discovery, UI, hardware drivers, and commissioning evidence are
future reviewed stages. Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`.
