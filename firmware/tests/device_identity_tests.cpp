#include <algorithm>
#include <cassert>
#include <string>
#include "zie/devices/DeviceIdentity.hpp"
namespace {
using namespace zie::devices;
using zie::extensions::TrustClass;
bool has(const IdentityValidationResult& r, IdentityIssue i) { return std::find(r.issues.begin(), r.issues.end(), i) != r.issues.end(); }
IdentityValidationContext assignment(ControllerIdentity controller = ControllerIdentity::esp32_s3, TrustClass trust = TrustClass::signed_trusted) { return {"zie.hardware.range", controller, "zie.full-prototype", trust}; }
DeviceIdentity device(std::string instance, std::string serial) { DeviceIdentity x; x.package.extension_id = "zie.hardware.range"; x.physical.state = PhysicalIdentityState::manufacturer_serial; x.physical.manufacturer = "STMicroelectronics"; x.physical.model = "VL53L1X"; x.physical.serial = std::move(serial); x.physical.provenance = {IdentitySource::manufacturer_attested, IdentityTrust::attested}; x.logical.instance_id = std::move(instance); x.controller.controller = ControllerIdentity::esp32_s3; x.hardware_profile.profile_id = "zie.full-prototype"; return x; }
}  // namespace
void run_device_identity_tests() {
  const auto trusted = assignment(); const auto first = device("head.range.primary", "sensor-0001"); assert(validate_identity(first, trusted).valid);
  auto provisional = first; provisional.physical = {}; provisional.physical.provisional_local_id = "local.range.0001"; provisional.physical.provenance = {IdentitySource::registry_generated, IdentityTrust::provisional}; assert(validate_identity(provisional, trusted).valid);
  auto second = device("head.range.backup", "sensor-0001"); auto result = validate_identity_set({first, second}, trusted); assert(!result.valid && has(result, IdentityIssue::duplicate_physical_identity));
  second = device("head.range.primary", "sensor-0002"); result = validate_identity_set({first, second}, trusted); assert(!result.valid && has(result, IdentityIssue::duplicate_logical_instance_identity));
  auto invalid = first; invalid.physical.provenance.source = IdentitySource::can_address; result = validate_identity(invalid, trusted); assert(!result.valid && has(result, IdentityIssue::ephemeral_stable_identity));
  invalid = first; invalid.physical.provenance.trust = IdentityTrust::provisional; result = validate_identity(invalid, trusted); assert(!result.valid && has(result, IdentityIssue::conflicting_provenance));
  // A candidate STM32 claim conflicts with the external ESP32 assignment even when
  // the registry-assigned extension trust is built-in.
  invalid = first; invalid.controller.controller = ControllerIdentity::stm32_safety; result = validate_identity(invalid, assignment(ControllerIdentity::esp32_s3, TrustClass::built_in)); assert(!result.valid && has(result, IdentityIssue::controller_assignment_mismatch));
  invalid = first; invalid.package.extension_id = "zie.hardware.impersonator"; result = validate_identity(invalid, trusted); assert(!result.valid && has(result, IdentityIssue::package_identity_mismatch));
  invalid = first; invalid.hardware_profile.profile_id = "zie.protected-production"; result = validate_identity(invalid, trusted); assert(!result.valid && has(result, IdentityIssue::hardware_profile_assignment_mismatch));
  auto safety = first; safety.controller.controller = ControllerIdentity::stm32_safety; assert(validate_identity(safety, assignment(ControllerIdentity::stm32_safety, TrustClass::built_in)).valid);
  invalid = provisional; invalid.physical.provenance.source = IdentitySource::extension_claim; result = validate_identity(invalid, trusted); assert(!result.valid && has(result, IdentityIssue::provisional_identity_requires_registry_source));
  auto active = first; active.lifecycle = IdentityLifecycle::active; auto changed = active; changed.logical.instance_id = "head.range.changed"; result = validate_identity_update(active, changed, trusted); assert(!result.valid && has(result, IdentityIssue::immutable_identity_after_activation));
  auto active_provisional = provisional; active_provisional.lifecycle = IdentityLifecycle::active; changed = active_provisional; changed.physical.manufacturer = "Changed manufacturer"; changed.physical.model = "Changed model"; result = validate_identity_update(active_provisional, changed, trusted); assert(!result.valid && has(result, IdentityIssue::immutable_identity_after_activation));
  invalid = first; invalid.physical.state = static_cast<PhysicalIdentityState>(999); invalid.physical.provenance.source = static_cast<IdentitySource>(999); invalid.logical.provenance.trust = static_cast<IdentityTrust>(999); invalid.controller.controller = static_cast<ControllerIdentity>(999); invalid.lifecycle = static_cast<IdentityLifecycle>(999); auto unknown = trusted; unknown.registry_controller = static_cast<ControllerIdentity>(999); unknown.registry_assigned_extension_trust = static_cast<TrustClass>(999); result = validate_identity(invalid, unknown); assert(!result.valid); assert(has(result, IdentityIssue::unknown_physical_identity_state)); assert(has(result, IdentityIssue::unknown_identity_source)); assert(has(result, IdentityIssue::unknown_identity_trust)); assert(has(result, IdentityIssue::unknown_controller_identity)); assert(has(result, IdentityIssue::unknown_lifecycle)); assert(has(result, IdentityIssue::unknown_extension_trust));
}
