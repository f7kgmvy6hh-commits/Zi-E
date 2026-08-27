# Device Identity Foundation

Status: in-memory C++17 validation contract, ready for review. It is not a registry,
loader, persistence format, discovery system, parser, or rebind implementation.

## Identity domains

The model deliberately keeps five domains distinct:

1. `ExtensionPackageIdentity` identifies the extension/package that supplies an
   integration. Registry-assigned extension trust remains external context.
2. `PhysicalDeviceIdentity` identifies hardware by manufacturer/model/serial, or by
   an explicit registry-generated provisional-local ID when no trustworthy serial
   exists.
3. `LogicalDeviceInstanceIdentity` is the stable registry-generated role/instance
   identity. It does not derive from display name or discovery order.
4. `ControllerBinding` states the candidate binding, but proves no authority. It must
   exactly match trusted registry context; protected STM32 safety identity additionally
   requires externally supplied built-in extension trust.
5. `HardwareProfileBinding` states the candidate profile binding and must exactly match
   trusted registry context. It is not a capability declaration or proof of assignment.

Identity contains no capability or permission fields. USB path, COM port, IP address,
CAN address, and discovery order are classified as ephemeral sources and rejected if
used as stable physical identity provenance. Display names and transport locations do
not appear in the stable model.

## Fail-closed invariants

- Every enum/domain is exhaustively checked; unknown values invalidate the record.
- Manufacturer identity requires manufacturer, model, serial, and matching attested
  or operator-verified provenance.
- Provisional identity requires a registry-generated local ID with provisional trust;
  it cannot masquerade as a hardware serial.
- Logical identity must be registry-generated and verified.
- Duplicate physical or logical identities invalidate the entire set.
- Identity fields and bindings cannot change after activation. A future explicit,
  audited migration/rebind workflow is required to replace them.
- Candidate package, controller, and profile fields must exactly match authoritative
  registry context. No candidate-carried authority flag exists.

## Deferred work

No JSON/file parsing, persistence, database, loader, USB/BLE/Wi-Fi enumeration, CAN
addressing, cloud account, UI, migration/rebind execution, capability validation, or
permission enforcement is included. No production hardware identifiers or CAN values
are selected here.
