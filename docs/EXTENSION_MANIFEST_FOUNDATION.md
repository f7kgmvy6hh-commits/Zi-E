# Extension Manifest Foundation

## Scope

This document describes the first host-testable contract for Zi-E extensions. The
implementation centralizes the in-memory model and validation invariants; it is not a
JSON schema, package installer, registry, loader, sandbox, signature verifier, device
identity registry, or configuration engine.

The implementation-dependent Phase 2B2 CAN path remains
`WAITING_FOR_VERIFIED_INPUTS`. Its evidence and freeze criteria remain in
`PHASE2B2_BUS_TIMING_ANALYSIS.md`.

## Extension execution classes

The classes are deliberately different trust/execution models:

| Class | Execution rule | Current validation |
|---|---|---|
| `HOST_PLUGIN` | Executable host/app adapter using later mediated host APIs | Must target host and declare an entrypoint; pack categories forbidden |
| `ASSET_PACK` | Data-only face/emotion/RGB/sound content | No controller target, entrypoint, or permissions; only pack categories |
| `EMBEDDED_MODULE` | Compile/link-time hardware module | Must target ESP32-S3 or STM32, use a hardware category, and request no host-plugin permissions |
| `PROTECTED_SAFETY_MODULE` | Built-in STM32 safety firmware only | STM32 hardware category, entrypoint, registry-assigned `BUILT_IN` trust, and no plugin permissions |

Trust is validation context assigned by the future registry/source. It is not a
publisher-controlled manifest field, so an extension cannot declare itself built-in or
signed. Permission entries are typed requests for a future enforcement layer, not
grants. `request_motion` means access only to the Safe Robot API; the vocabulary has no
raw GPIO, PWM, register, actuator-setpoint, or safety-bypass permission.

## Independently versioned contracts

The model separates:

- extension package version;
- Plugin API version;
- manifest schema version.

The initial supported Plugin API and manifest schema are both `1.0.0`, but they are
independent values. Validation requires a matching nonzero major and a required minor
no newer than the supported minor. Serialization and migration rules remain future
schema work; this C++ model does not claim to parse untrusted package files.

## Categories

The typed catalog contains the approved logical categories from `hardware.display`
through `integration`. Code uses enum names such as `hardware_display`; a future schema
adapter owns the canonical dotted string mapping so structure is not scattered through
core services.

## Capability trust invariant

The contract keeps three separate sets:

```text
declared_capabilities    publisher claim in a validated manifest
validated_capabilities  capabilities proven by class-specific validation/commissioning
active_capabilities     validated capabilities currently safe and available to resolve
```

Validation enforces:

- every enum-bearing input is a known supported class, category, permission, trust,
  controller target, lifecycle state, or failure class before semantic validation;
- declared keys are syntactically valid and unique;
- validated capabilities are a subset of declared capabilities;
- active capabilities are a subset of validated capabilities;
- only `ACTIVE` or `DEGRADED` lifecycle states may expose active capabilities;
- `FAILED` requires a failure class, while failure metadata is limited to
  `DEGRADED`, `FAILED`, or `QUARANTINED` states.

A valid manifest starts with no validated or active capabilities. Installation,
configuration, health checks, and commissioning must populate later state explicitly.

## Lifecycle and failures

Lifecycle values are `DISCOVERED`, `VALIDATING`, `INSTALLED`, `VALIDATED`, `INACTIVE`,
`ACTIVATING`, `CONFIGURED`, `COMMISSIONING`, `ACTIVE`, `DEGRADED`, `FAILED`,
`QUARANTINED`, `DISABLED`, and `REMOVED`. Failure classes are `TEMPORARY`, `CONFIGURATION`, `AUTHENTICATION`,
`INCOMPATIBLE`, `HARDWARE_FAULT`, and `SECURITY`, plus `NONE` when no failure applies.

The later in-memory registry foundation now authorizes explicit transitions and owns
capability revocation. This manifest layer still validates snapshots independently.

## Validation coverage

Host tests cover valid representatives of all four classes and reject:

- incompatible Plugin API or manifest schema;
- duplicate/invalid capability declarations;
- executable or permission-bearing asset packs;
- class/category/controller mismatch;
- non-built-in protected safety modules;
- duplicate permission requests;
- undeclared validation and unvalidated activation;
- active capabilities in inactive lifecycle states;
- inconsistent lifecycle/failure metadata.
- adversarial out-of-domain enum values for every trust/validation enum.

Enum validation is explicit and fail-closed. Hardware/asset category classification is
exhaustive and does not depend on enum declaration order. There is no `unknown`
permission value that a later enforcement layer could accidentally grant.

## Deferred work

- canonical JSON schema, parser, size/depth limits, and unknown-field policy;
- configuration schemas, secret references, migration, staging, and rollback;
- stable physical-device/plugin/instance/controller/bus identities;
- dependency/conflict resolution and cycle detection;
- persistent registry storage, migration, and crash-atomic transactions;
- actual host permission enforcement/sandboxing;
- signatures, publisher identity, revocation, and provenance;
- embedded composition-root generation and commissioning evidence;
- asset file integrity, resource budgets, and protected system fallback.

No extension becomes loadable or executable merely because this model validates it.
