#pragma once
#include <string>
#include <vector>
#include "zie/extensions/ExtensionManifest.hpp"
namespace zie::devices {
enum class PhysicalIdentityState { manufacturer_serial, provisional_local };
enum class IdentitySource { manufacturer_attested, operator_verified, registry_generated, extension_claim, usb_path, com_port, ip_address, can_address, discovery_order };
enum class IdentityTrust { attested, verified, provisional };
enum class ControllerIdentity { none, host, esp32_s3, stm32_safety };
enum class IdentityLifecycle { configured, active };
struct IdentityProvenance { IdentitySource source{IdentitySource::extension_claim}; IdentityTrust trust{IdentityTrust::provisional}; };
struct ExtensionPackageIdentity { std::string extension_id; };
struct PhysicalDeviceIdentity { PhysicalIdentityState state{PhysicalIdentityState::provisional_local}; std::string manufacturer; std::string model; std::string serial; std::string provisional_local_id; IdentityProvenance provenance{}; };
struct LogicalDeviceInstanceIdentity { std::string instance_id; IdentityProvenance provenance{IdentitySource::registry_generated, IdentityTrust::verified}; };
struct ControllerBinding { ControllerIdentity controller{ControllerIdentity::none}; };
struct HardwareProfileBinding { std::string profile_id; };
// Contains identity and bindings only: never capabilities or permissions. Extension
// trust is supplied by registry context and cannot be claimed by this record.
struct DeviceIdentity { ExtensionPackageIdentity package; PhysicalDeviceIdentity physical; LogicalDeviceInstanceIdentity logical; ControllerBinding controller; HardwareProfileBinding hardware_profile; IdentityLifecycle lifecycle{IdentityLifecycle::configured}; };
// This context is supplied by the trusted registry boundary. Candidate records cannot
// manufacture registry authority by changing their own fields.
struct IdentityValidationContext { std::string registry_extension_id; ControllerIdentity registry_controller{ControllerIdentity::none}; std::string registry_hardware_profile_id; extensions::TrustClass registry_assigned_extension_trust{extensions::TrustClass::community_untrusted}; };
enum class IdentityIssue { invalid_extension_id, invalid_manufacturer, invalid_model, invalid_serial, invalid_provisional_local_id, invalid_logical_instance_id, invalid_hardware_profile_id, unknown_physical_identity_state, unknown_identity_source, unknown_identity_trust, unknown_controller_identity, unknown_lifecycle, unknown_extension_trust, ephemeral_stable_identity, conflicting_provenance, manufacturer_identity_has_provisional_id, provisional_identity_has_hardware_serial, provisional_identity_requires_registry_source, logical_identity_requires_registry_assignment, package_identity_mismatch, controller_assignment_mismatch, protected_controller_requires_built_in_trust, hardware_profile_assignment_mismatch, duplicate_physical_identity, duplicate_logical_instance_identity, immutable_identity_after_activation };
struct IdentityValidationResult { bool valid{false}; std::vector<IdentityIssue> issues; };
IdentityValidationResult validate_identity(const DeviceIdentity&, const IdentityValidationContext&);
IdentityValidationResult validate_identity_set(const std::vector<DeviceIdentity>&, const IdentityValidationContext&);
IdentityValidationResult validate_identity_update(const DeviceIdentity&, const DeviceIdentity&, const IdentityValidationContext&);
}  // namespace zie::devices
