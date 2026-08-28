#include "zie/extensions/ExtensionRegistry.hpp"

#include <algorithm>
#include <cctype>
#include <limits>

namespace zie::extensions {
namespace {

template <typename T>
bool contains(const std::vector<T>& values, const T& value) {
  return std::find(values.begin(), values.end(), value) != values.end();
}

bool known(const LifecycleState value) {
  switch (value) {
    case LifecycleState::discovered:
    case LifecycleState::validating:
    case LifecycleState::installed:
    case LifecycleState::validated:
    case LifecycleState::inactive:
    case LifecycleState::activating:
    case LifecycleState::configured:
    case LifecycleState::commissioning:
    case LifecycleState::active:
    case LifecycleState::degraded:
    case LifecycleState::failed:
    case LifecycleState::quarantined:
    case LifecycleState::disabled:
    case LifecycleState::removed:
      return true;
  }
  return false;
}

bool known(const FailureClass value) {
  switch (value) {
    case FailureClass::none:
    case FailureClass::temporary:
    case FailureClass::configuration:
    case FailureClass::authentication:
    case FailureClass::incompatible:
    case FailureClass::hardware_fault:
    case FailureClass::security:
      return true;
  }
  return false;
}

bool failure_state(const LifecycleState state) {
  return state == LifecycleState::degraded ||
         state == LifecycleState::failed ||
         state == LifecycleState::quarantined;
}

bool legal_transition(const LifecycleState from, const LifecycleState to) {
  switch (from) {
    case LifecycleState::installed:
      return to == LifecycleState::validated || to == LifecycleState::failed ||
             to == LifecycleState::quarantined ||
             to == LifecycleState::disabled || to == LifecycleState::removed;
    case LifecycleState::validated:
      return to == LifecycleState::inactive || to == LifecycleState::failed ||
             to == LifecycleState::quarantined ||
             to == LifecycleState::disabled || to == LifecycleState::removed;
    case LifecycleState::inactive:
      return to == LifecycleState::activating ||
             to == LifecycleState::quarantined ||
             to == LifecycleState::disabled || to == LifecycleState::removed;
    case LifecycleState::activating:
      return to == LifecycleState::inactive || to == LifecycleState::failed ||
             to == LifecycleState::quarantined ||
             to == LifecycleState::disabled || to == LifecycleState::removed;
    case LifecycleState::active:
      return to == LifecycleState::inactive ||
             to == LifecycleState::degraded || to == LifecycleState::failed ||
             to == LifecycleState::quarantined ||
             to == LifecycleState::disabled || to == LifecycleState::removed;
    case LifecycleState::degraded:
      return to == LifecycleState::inactive || to == LifecycleState::failed ||
             to == LifecycleState::quarantined ||
             to == LifecycleState::disabled || to == LifecycleState::removed;
    case LifecycleState::failed:
      return to == LifecycleState::inactive ||
             to == LifecycleState::quarantined ||
             to == LifecycleState::disabled || to == LifecycleState::removed;
    case LifecycleState::quarantined:
      return to == LifecycleState::disabled || to == LifecycleState::removed;
    case LifecycleState::disabled:
      return to == LifecycleState::inactive || to == LifecycleState::removed;
    case LifecycleState::removed:
    case LifecycleState::discovered:
    case LifecycleState::validating:
    case LifecycleState::configured:
    case LifecycleState::commissioning:
      return false;
  }
  return false;
}

bool same_physical_key(const devices::PhysicalDeviceIdentity& left,
                       const devices::PhysicalDeviceIdentity& right) {
  if (left.state != right.state) return false;
  if (left.state == devices::PhysicalIdentityState::manufacturer_serial) {
    return left.manufacturer == right.manufacturer &&
           left.model == right.model && left.serial == right.serial;
  }
  return left.provisional_local_id == right.provisional_local_id;
}

bool conflicts(const ExtensionRecord& record,
               const devices::DeviceIdentity& identity) {
  return same_physical_key(record.device_identity.physical, identity.physical) ||
         record.device_identity.logical.instance_id == identity.logical.instance_id;
}

bool known(const SemanticAttributeDomain value) {
  switch (value) {
    case SemanticAttributeDomain::variant:
    case SemanticAttributeDomain::placement:
    case SemanticAttributeDomain::performance_class:
    case SemanticAttributeDomain::tool_class:
      return true;
  }
  return false;
}

bool valid_semantic_value(const std::string& value) {
  if (value.empty() ||
      !std::isalnum(static_cast<unsigned char>(value.front()))) {
    return false;
  }
  return std::all_of(value.begin(), value.end(), [](const unsigned char part) {
    return std::islower(part) || std::isdigit(part) || part == '.' ||
           part == '_' || part == '-';
  });
}

bool valid_attributes(const std::vector<SemanticDeviceAttribute>& attributes) {
  for (std::size_t index = 0; index < attributes.size(); ++index) {
    if (!known(attributes[index].domain) ||
        !valid_semantic_value(attributes[index].value)) {
      return false;
    }
    for (std::size_t other = index + 1; other < attributes.size(); ++other) {
      if (attributes[index].domain == attributes[other].domain) return false;
    }
  }
  return true;
}

}  // namespace

bool ExtensionRegistry::assign_new_authorization_generation(
    ExtensionRecord& record) {
  if (next_authorization_generation_ == 0 ||
      next_authorization_generation_ ==
          std::numeric_limits<std::uint64_t>::max()) {
    return false;
  }
  record.authorization_generation = next_authorization_generation_++;
  return true;
}

RegistryResult ExtensionRegistry::register_extension(
    const ExtensionCandidate& candidate, const RegistryAssignment& assignment) {
  if (candidate.manifest.id != assignment.package_id ||
      candidate.device_identity.package.extension_id != assignment.package_id) {
    return RegistryResult::rejected_package_mismatch;
  }
  if (!valid_attributes(assignment.semantic_attributes)) {
    return RegistryResult::rejected_invalid_identity;
  }
  const auto* existing = find(assignment.package_id);
  if (existing != nullptr && existing->lifecycle != LifecycleState::removed) {
    return RegistryResult::rejected_duplicate_package;
  }

  const auto manifest_result = validate_manifest(
      candidate.manifest, ManifestValidationContext{assignment.assigned_trust});
  if (!manifest_result.valid) return RegistryResult::rejected_invalid_manifest;

  const devices::IdentityValidationContext identity_context{
      assignment.package_id, assignment.controller,
      assignment.hardware_profile_id, assignment.assigned_trust};
  if (!devices::validate_identity(candidate.device_identity, identity_context).valid) {
    return RegistryResult::rejected_invalid_identity;
  }
  for (const auto& record : records_) {
    if (record.lifecycle != LifecycleState::removed &&
        conflicts(record, candidate.device_identity)) {
      return RegistryResult::rejected_identity_conflict;
    }
  }

  ExtensionRecord record;
  record.manifest = candidate.manifest;
  record.device_identity = candidate.device_identity;
  record.assigned_trust = assignment.assigned_trust;
  record.semantic_attributes = assignment.semantic_attributes;
  if (existing != nullptr &&
      existing->device_identity.logical.instance_id ==
          candidate.device_identity.logical.instance_id) {
    return RegistryResult::rejected_identity_conflict;
  }
  if (!assign_new_authorization_generation(record)) {
    return RegistryResult::rejected_generation_exhausted;
  }
  if (existing != nullptr) {
    *find_mutable(assignment.package_id) = std::move(record);
  } else {
    records_.push_back(std::move(record));
  }
  return RegistryResult::accepted;
}

RegistryResult ExtensionRegistry::unregister_extension(
    const std::string& package_id) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return RegistryResult::rejected_not_found;
  if (record->lifecycle == LifecycleState::removed) {
    return RegistryResult::rejected_removed;
  }
  if (!assign_new_authorization_generation(*record)) {
    return RegistryResult::rejected_generation_exhausted;
  }
  record->active_capabilities.clear();
  record->failure = FailureClass::none;
  record->lifecycle = LifecycleState::removed;
  return RegistryResult::removed;
}

RegistryResult ExtensionRegistry::transition(const std::string& package_id,
                                             const LifecycleState target,
                                             const FailureClass failure) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return RegistryResult::rejected_not_found;
  if (record->lifecycle == LifecycleState::removed) {
    return RegistryResult::rejected_removed;
  }
  if (!known(target) || !known(failure)) {
    return RegistryResult::rejected_unknown_lifecycle;
  }
  if (target == LifecycleState::active) {
    return RegistryResult::rejected_illegal_transition;
  }
  if (!legal_transition(record->lifecycle, target) ||
      (failure_state(target) != (failure != FailureClass::none))) {
    return RegistryResult::rejected_illegal_transition;
  }
  if (target != LifecycleState::active && target != LifecycleState::degraded &&
      !assign_new_authorization_generation(*record)) {
    return RegistryResult::rejected_generation_exhausted;
  }
  record->lifecycle = target;
  record->failure = failure;
  if (target != LifecycleState::active && target != LifecycleState::degraded) {
    record->active_capabilities.clear();
  }
  return target == LifecycleState::removed ? RegistryResult::removed
                                            : RegistryResult::transitioned;
}

RegistryResult ExtensionRegistry::validate_capabilities(
    const std::string& package_id,
    const std::vector<std::string>& validated_capabilities) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return RegistryResult::rejected_not_found;
  if (record->lifecycle == LifecycleState::removed) {
    return RegistryResult::rejected_removed;
  }
  if (record->lifecycle != LifecycleState::validated) {
    return RegistryResult::rejected_illegal_transition;
  }
  CapabilityState state;
  state.lifecycle = LifecycleState::inactive;
  state.validated_capabilities = validated_capabilities;
  if (!validate_capability_state(record->manifest, state).valid) {
    return RegistryResult::rejected_capability_state;
  }
  record->validated_capabilities = validated_capabilities;
  return RegistryResult::capabilities_validated;
}

RegistryResult ExtensionRegistry::activate_capabilities(
    const std::string& package_id,
    const std::vector<std::string>& active_capabilities) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return RegistryResult::rejected_not_found;
  if (record->lifecycle == LifecycleState::removed) {
    return RegistryResult::rejected_removed;
  }
  if (record->lifecycle != LifecycleState::activating) {
    return RegistryResult::rejected_illegal_transition;
  }
  CapabilityState state;
  state.lifecycle = LifecycleState::active;
  state.validated_capabilities = record->validated_capabilities;
  state.active_capabilities = active_capabilities;
  if (!validate_capability_state(record->manifest, state).valid) {
    return RegistryResult::rejected_capability_state;
  }
  for (const auto& capability : active_capabilities) {
    if (!resolve(capability).empty()) {
      return RegistryResult::rejected_ambiguous_capability;
    }
  }
  if (!assign_new_authorization_generation(*record)) {
    return RegistryResult::rejected_generation_exhausted;
  }
  record->active_capabilities = active_capabilities;
  record->lifecycle = LifecycleState::active;
  record->failure = FailureClass::none;
  return RegistryResult::capabilities_activated;
}

RegistryResult ExtensionRegistry::replace_package_version(
    const std::string& package_id, const ContractVersion version,
    const std::uint64_t expected_authorization_generation) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return RegistryResult::rejected_not_found;
  if (record->lifecycle == LifecycleState::removed) {
    return RegistryResult::rejected_removed;
  }
  if (record->lifecycle != LifecycleState::inactive ||
      record->authorization_generation != expected_authorization_generation ||
      version.major == 0) {
    return RegistryResult::rejected_illegal_transition;
  }
  if (!assign_new_authorization_generation(*record)) {
    return RegistryResult::rejected_generation_exhausted;
  }
  record->manifest.version = version;
  return RegistryResult::package_version_replaced;
}

const ExtensionRecord* ExtensionRegistry::find(
    const std::string& package_id) const {
  const auto found = std::find_if(records_.begin(), records_.end(),
                                  [&package_id](const ExtensionRecord& record) {
                                    return record.manifest.id == package_id;
                                  });
  return found == records_.end() ? nullptr : &*found;
}

ExtensionRecord* ExtensionRegistry::find_mutable(const std::string& package_id) {
  const auto found = std::find_if(records_.begin(), records_.end(),
                                  [&package_id](const ExtensionRecord& record) {
                                    return record.manifest.id == package_id;
                                  });
  return found == records_.end() ? nullptr : &*found;
}

std::vector<CapabilityProvider> ExtensionRegistry::resolve(
    const std::string& capability) const {
  std::vector<CapabilityProvider> providers;
  for (const auto& record : records_) {
    if ((record.lifecycle == LifecycleState::active ||
         record.lifecycle == LifecycleState::degraded) &&
        contains(record.active_capabilities, capability)) {
      providers.push_back(
          {record.manifest.id, record.device_identity.logical.instance_id});
    }
  }
  return providers;
}

std::vector<CapabilityCandidate> ExtensionRegistry::inspect_capability(
    const std::string& capability) const {
  std::vector<CapabilityCandidate> candidates;
  for (const auto& record : records_) {
    for (const auto& declared : record.manifest.declared_capabilities) {
      const bool exact = declared == capability;
      const bool semantic_hardware_variant =
          capability.compare(0, 9, "hardware.") == 0 &&
          declared.compare(0, capability.size(), capability) == 0 &&
          declared.size() > capability.size() &&
          declared[capability.size()] == '.';
      if (!exact && !semantic_hardware_variant) continue;
      candidates.push_back(
          {record.manifest.id,
           record.device_identity.logical.instance_id,
           record.device_identity.hardware_profile.profile_id,
           declared,
           record.semantic_attributes,
           record.manifest.category,
           record.manifest.extension_class,
           record.assigned_trust,
           record.device_identity.controller.controller,
           record.lifecycle,
           record.authorization_generation,
           contains(record.validated_capabilities, declared),
           contains(record.active_capabilities, declared)});
    }
  }
  return candidates;
}

}  // namespace zie::extensions
