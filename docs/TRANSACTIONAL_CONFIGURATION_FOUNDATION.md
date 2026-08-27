# Transactional Configuration Foundation

Status: in-memory C++17 validation and snapshot contract. It has no parser,
persistence, filesystem monitoring, UI, cloud synchronization, loader, or hardware
application mechanism.

## Model

Configuration is separated into:

- declared keys, value domains, and semantic effect classification;
- one staged candidate revision;
- one validated candidate snapshot;
- one immutable active snapshot;
- registry-controlled commit generation and candidate-supplied monotonic revision.

The trusted request context, candidate binding, stored declaration owner, and live
registry record must agree on package, logical device instance, and hardware profile.
Stale/nonzero revision checks occur before staging. Validation checks unique declared
keys, known domains, declaration/type matching, typed values, and forbidden
authority/raw-control namespaces. `bounded_integer` declarations require signed
64-bit inclusive minimum and maximum with `minimum <= maximum`; values use exact
base-10 `from_chars` parsing and reject malformed, trailing, overflow, underflow, and
out-of-range input. Booleans accept only canonical lowercase `true` or `false`.

## Commit and rollback

Only a currently staged revision that has been validated unchanged can commit. The
commit constructs the complete next snapshot, increments its generation, and replaces
the active optional snapshot in one in-memory model update. Any failed stage,
validation, lifecycle gate, stale revision, or commit leaves the previous active
snapshot unchanged. Explicit rollback discards staged and validated candidates without
touching active configuration.

Commit is allowed only while the owning extension is `inactive`, `active`, or
`degraded`. Quarantined, disabled, failed, installed, activating, and removed records
cannot activate a configuration. A retained active snapshot is historical model state;
it provides no capability exposure and cannot override registry lifecycle.

## Authority boundary

Configuration grants no trust, permission, capability, controller ownership, profile
ownership, or safety authority. Reserved namespaces for those claims and for raw GPIO,
PWM, registers, actuators, drivers, safety bypass, and CAN are rejected. A
`semantic_hardware` declaration is only a high-level setting to be interpreted later by
an authorized, commissioned service; this foundation applies nothing to hardware.

## Deferred work

Schema serialization, JSON/YAML parsing, persistence/crash recovery, migration, secret
references, filesystem watchers, UI, cloud sync, dynamic loading, runtime permission
enforcement, configuration application callbacks, hardware rollback, and distributed
transactions are future stages. Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`.
