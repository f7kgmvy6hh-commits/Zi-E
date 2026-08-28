# Hardware Profile and Resolution Foundation

Status: in-memory GNU C++17 contract and deterministic host tests. This stage defines
profile identity, validation, resolution, activation, and revocation behavior. It does
not discover hardware, load profiles, bind drivers, or commission a robot.

## Identity and authority

A profile has a stable semantic ID, exact schema contract version (`1.0.0`), positive
catalog revision, platform class, authoritative owner/source, and a host-assigned
resolution generation. The trusted `HardwareProfileAuthority` is supplied separately
from profile content so a candidate cannot claim ownership or impersonate another
profile. Device candidates retain the existing registry-authoritative package,
logical-device, controller, trust, and exact hardware-profile binding.

Transport paths, COM ports, IP addresses, discovery order, bus addresses, and raw pin
maps are not identity. Unknown schema, enum, or authority domains fail closed.

## Semantic profile entries

Entries describe display, camera, audio input/output, range/proximity/cliff, IMU,
drive, arm, gripper, tool, RGB, touch, power, battery, and network needs. Each entry
contains a unique logical slot, closed semantic capability, compatible extension
category, required/optional policy, cardinality, ownership domain, and bounded safe
semantic constraints.

The profile contains no GPIO, PWM, register, setpoint, motor, actuator, CAN, USB,
address, or protected-safety bypass configuration. Raw namespaces are rejected in
slots, capabilities, and constraint values. Protected ownership is limited to the
safety-relevant cliff, drive, power, and battery domains and can resolve only to a
built-in protected safety module bound to the STM32 safety controller.

## Validation and resolution

Validation rejects unknown domains/categories/cardinalities, duplicate logical slots
or capabilities, conflicting requirements, impossible required cardinality, category
mismatch, invalid authority, unsafe protected ownership, and raw-control namespaces.
The schema match is exact for this foundation; unknown future versions are not guessed
compatible.

Resolution reads copied candidate facts from `ExtensionRegistry`. Matching is sorted
by package and logical-device identity and filters, in order, by exact profile binding,
compatible category, active lifecycle plus validated/active capability, and ownership.
Single-cardinality ambiguity fails closed. Optional absence is recorded without making
the profile unready; all other missing, inactive, wrong-profile, incompatible,
protected-ownership, or ambiguous outcomes preserve a typed status and diagnostic
package where one exists. Results and match counts are bounded and returned by value.

The registry continues to own lifecycle, trust, controller, identity, and capability
authority, including its unique concrete-capability activation rule. A profile may
match distinct concrete semantic variants under the same closed hardware domain; the
resolver rejects a single-cardinality entry if more than one variant is eligible.

## Activation and revocation

The lifecycle is `declared -> validated -> resolved -> active`. Activation requires a
ready resolution and rechecks every bound package, logical device, profile,
authorization generation, lifecycle, active capability, and category. Replacement is
atomic: a failed candidate activation leaves the previous active profile unchanged.

If a selected extension is revoked or its generation/binding changes, the active
resolution becomes invalidated and the active selection is cleared. The resolver does
not silently choose another matching device. Recovery or activation of a spare still
requires an explicit re-resolution followed by explicit profile activation, which
receives a fresh profile generation.

The Plugin SDK remains downstream of this authority. Its host-issued context reports
the registry-approved hardware-profile binding; a plugin cannot alter that value.
Transactional configuration cannot grant a profile, trust, capability, identity, or
raw hardware authority.

## Validation scope

Host tests cover minimal and optional-full profiles, duplicate and unknown inputs,
missing/ambiguous/wrong-profile/inactive/quarantined devices, identity impersonation,
atomic replacement, revocation without silent rebinding, explicit re-resolution,
authoritative SDK profile identity, protected ownership, and raw namespace rejection.

Excluded: USB/BLE/Wi-Fi enumeration, CAN addressing/framing, real drivers, filesystem
parsers, persistence, UI editing, probing, board pin maps, commissioning, dynamic
loading, transport, and physical hardware validation. Phase 2B2 remains
`WAITING_FOR_VERIFIED_INPUTS`.
