#include "zie/devices/DeviceIdentity.hpp"
#include <algorithm>
#include <cctype>
namespace zie::devices { namespace {
template <typename T> bool contains(const std::vector<T>& v, const T x) { return std::find(v.begin(), v.end(), x) != v.end(); }
void add(IdentityValidationResult& r, IdentityIssue i) { if (!contains(r.issues, i)) r.issues.push_back(i); }
bool valid_key(const std::string& v) { if (v.empty() || !std::isalnum(static_cast<unsigned char>(v.front()))) return false; return std::all_of(v.begin(), v.end(), [](unsigned char c) { return std::islower(c) || std::isdigit(c) || c == '.' || c == '_' || c == '-'; }); }
bool known(PhysicalIdentityState v) { switch (v) { case PhysicalIdentityState::manufacturer_serial: case PhysicalIdentityState::provisional_local: return true; } return false; }
bool known(IdentitySource v) { switch (v) { case IdentitySource::manufacturer_attested: case IdentitySource::operator_verified: case IdentitySource::registry_generated: case IdentitySource::extension_claim: case IdentitySource::usb_path: case IdentitySource::com_port: case IdentitySource::ip_address: case IdentitySource::can_address: case IdentitySource::discovery_order: return true; } return false; }
bool known(IdentityTrust v) { switch (v) { case IdentityTrust::attested: case IdentityTrust::verified: case IdentityTrust::provisional: return true; } return false; }
bool known(ControllerIdentity v) { switch (v) { case ControllerIdentity::none: case ControllerIdentity::host: case ControllerIdentity::esp32_s3: case ControllerIdentity::stm32_safety: return true; } return false; }
bool known(IdentityLifecycle v) { switch (v) { case IdentityLifecycle::configured: case IdentityLifecycle::active: return true; } return false; }
bool known(extensions::TrustClass v) { switch (v) { case extensions::TrustClass::built_in: case extensions::TrustClass::signed_trusted: case extensions::TrustClass::local_developer: case extensions::TrustClass::community_untrusted: return true; } return false; }
bool ephemeral(IdentitySource v) { switch (v) { case IdentitySource::usb_path: case IdentitySource::com_port: case IdentitySource::ip_address: case IdentitySource::can_address: case IdentitySource::discovery_order: return true; case IdentitySource::manufacturer_attested: case IdentitySource::operator_verified: case IdentitySource::registry_generated: case IdentitySource::extension_claim: return false; } return true; }
bool same_physical_key(const PhysicalDeviceIdentity& a, const PhysicalDeviceIdentity& b) { if (a.state != b.state) return false; return a.state == PhysicalIdentityState::manufacturer_serial ? a.manufacturer == b.manufacturer && a.model == b.model && a.serial == b.serial : a.provisional_local_id == b.provisional_local_id; }
bool same_physical_record(const PhysicalDeviceIdentity& a, const PhysicalDeviceIdentity& b) { return a.state == b.state && a.manufacturer == b.manufacturer && a.model == b.model && a.serial == b.serial && a.provisional_local_id == b.provisional_local_id && a.provenance.source == b.provenance.source && a.provenance.trust == b.provenance.trust; }
bool same_immutable(const DeviceIdentity& a, const DeviceIdentity& b) { return a.package.extension_id == b.package.extension_id && same_physical_record(a.physical, b.physical) && a.logical.instance_id == b.logical.instance_id && a.logical.provenance.source == b.logical.provenance.source && a.logical.provenance.trust == b.logical.provenance.trust && a.controller.controller == b.controller.controller && a.hardware_profile.profile_id == b.hardware_profile.profile_id; }
}  // namespace
IdentityValidationResult validate_identity(const DeviceIdentity& x, const IdentityValidationContext& c) {
  IdentityValidationResult r; const bool ks = known(x.physical.state), kps = known(x.physical.provenance.source), kpt = known(x.physical.provenance.trust), kls = known(x.logical.provenance.source), klt = known(x.logical.provenance.trust);
  if (!ks) add(r, IdentityIssue::unknown_physical_identity_state);
  if (!kps || !kls) add(r, IdentityIssue::unknown_identity_source);
  if (!kpt || !klt) add(r, IdentityIssue::unknown_identity_trust);
  if (!known(x.controller.controller))
    add(r, IdentityIssue::unknown_controller_identity);
  if (!known(c.registry_controller))
    add(r, IdentityIssue::unknown_controller_identity);
  if (!known(x.lifecycle)) add(r, IdentityIssue::unknown_lifecycle);
  if (!known(c.registry_assigned_extension_trust))
    add(r, IdentityIssue::unknown_extension_trust);
  if (!valid_key(x.package.extension_id))
    add(r, IdentityIssue::invalid_extension_id);
  if (!valid_key(c.registry_extension_id) ||
      x.package.extension_id != c.registry_extension_id)
    add(r, IdentityIssue::package_identity_mismatch);
  if (!valid_key(x.logical.instance_id))
    add(r, IdentityIssue::invalid_logical_instance_id);
  if (!valid_key(x.hardware_profile.profile_id))
    add(r, IdentityIssue::invalid_hardware_profile_id);
  if (!valid_key(c.registry_hardware_profile_id) ||
      x.hardware_profile.profile_id != c.registry_hardware_profile_id)
    add(r, IdentityIssue::hardware_profile_assignment_mismatch);
  if (kps && ephemeral(x.physical.provenance.source))
    add(r, IdentityIssue::ephemeral_stable_identity);
  if (ks && x.physical.state == PhysicalIdentityState::manufacturer_serial) { if (x.physical.manufacturer.empty()) add(r, IdentityIssue::invalid_manufacturer); if (x.physical.model.empty()) add(r, IdentityIssue::invalid_model); if (x.physical.serial.empty()) add(r, IdentityIssue::invalid_serial); if (!x.physical.provisional_local_id.empty()) add(r, IdentityIssue::manufacturer_identity_has_provisional_id); if (kps && kpt && !((x.physical.provenance.source == IdentitySource::manufacturer_attested && x.physical.provenance.trust == IdentityTrust::attested) || (x.physical.provenance.source == IdentitySource::operator_verified && x.physical.provenance.trust == IdentityTrust::verified))) add(r, IdentityIssue::conflicting_provenance); }
  if (ks && x.physical.state == PhysicalIdentityState::provisional_local) { if (!valid_key(x.physical.provisional_local_id)) add(r, IdentityIssue::invalid_provisional_local_id); if (!x.physical.serial.empty()) add(r, IdentityIssue::provisional_identity_has_hardware_serial); if (kps && kpt && (x.physical.provenance.source != IdentitySource::registry_generated || x.physical.provenance.trust != IdentityTrust::provisional)) add(r, IdentityIssue::provisional_identity_requires_registry_source); }
  if (kls && klt &&
      (x.logical.provenance.source != IdentitySource::registry_generated ||
       x.logical.provenance.trust != IdentityTrust::verified))
    add(r, IdentityIssue::logical_identity_requires_registry_assignment);
  if (known(x.controller.controller) && known(c.registry_controller) &&
      x.controller.controller != c.registry_controller)
    add(r, IdentityIssue::controller_assignment_mismatch);
  if (x.controller.controller == ControllerIdentity::stm32_safety &&
      c.registry_controller == ControllerIdentity::stm32_safety &&
      c.registry_assigned_extension_trust != extensions::TrustClass::built_in)
    add(r, IdentityIssue::protected_controller_requires_built_in_trust);
  r.valid = r.issues.empty(); return r;
}
IdentityValidationResult validate_identity_set(const std::vector<DeviceIdentity>& xs, const IdentityValidationContext& c) { IdentityValidationResult r; for (const auto& x : xs) { const auto one = validate_identity(x, c); for (auto i : one.issues) add(r, i); } for (std::size_t a = 0; a < xs.size(); ++a) for (std::size_t b = a + 1; b < xs.size(); ++b) { if (same_physical_key(xs[a].physical, xs[b].physical)) add(r, IdentityIssue::duplicate_physical_identity); if (xs[a].logical.instance_id == xs[b].logical.instance_id) add(r, IdentityIssue::duplicate_logical_instance_identity); } r.valid = r.issues.empty(); return r; }
IdentityValidationResult validate_identity_update(const DeviceIdentity& current, const DeviceIdentity& proposed, const IdentityValidationContext& c) { auto r = validate_identity(proposed, c); if (current.lifecycle == IdentityLifecycle::active && !same_immutable(current, proposed)) add(r, IdentityIssue::immutable_identity_after_activation); r.valid = r.issues.empty(); return r; }
}  // namespace zie::devices
