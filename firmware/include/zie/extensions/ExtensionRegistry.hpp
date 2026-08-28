#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "zie/devices/DeviceIdentity.hpp"
#include "zie/extensions/ExtensionManifest.hpp"

namespace zie::extensions {

enum class SemanticAttributeDomain {
  variant,
  placement,
  performance_class,
  tool_class,
};

struct SemanticDeviceAttribute {
  SemanticAttributeDomain domain{SemanticAttributeDomain::variant};
  std::string value;
};

struct RegistryAssignment {
  std::string package_id;
  TrustClass assigned_trust{TrustClass::community_untrusted};
  devices::ControllerIdentity controller{devices::ControllerIdentity::none};
  std::string hardware_profile_id;
  std::vector<SemanticDeviceAttribute> semantic_attributes;
};

struct ExtensionCandidate {
  ExtensionManifest manifest;
  devices::DeviceIdentity device_identity;
};

struct ExtensionRecord {
  ExtensionManifest manifest;
  devices::DeviceIdentity device_identity;
  TrustClass assigned_trust{TrustClass::community_untrusted};
  LifecycleState lifecycle{LifecycleState::installed};
  FailureClass failure{FailureClass::none};
  std::uint64_t authorization_generation{0};
  std::vector<std::string> validated_capabilities;
  std::vector<std::string> active_capabilities;
  std::vector<SemanticDeviceAttribute> semantic_attributes;
};

struct CapabilityProvider {
  std::string package_id;
  std::string logical_device_instance_id;
};

struct CapabilityCandidate {
  std::string package_id;
  std::string logical_device_instance_id;
  std::string hardware_profile_id;
  std::string capability;
  std::vector<SemanticDeviceAttribute> semantic_attributes;
  ExtensionCategory category{ExtensionCategory::behavior};
  ExtensionClass extension_class{ExtensionClass::host_plugin};
  TrustClass assigned_trust{TrustClass::community_untrusted};
  devices::ControllerIdentity controller{devices::ControllerIdentity::none};
  LifecycleState lifecycle{LifecycleState::discovered};
  std::uint64_t authorization_generation{0};
  bool capability_validated{false};
  bool capability_active{false};
};

enum class RegistryResult {
  accepted,
  transitioned,
  capabilities_validated,
  capabilities_activated,
  package_version_replaced,
  removed,
  rejected_invalid_manifest,
  rejected_invalid_identity,
  rejected_package_mismatch,
  rejected_duplicate_package,
  rejected_identity_conflict,
  rejected_not_found,
  rejected_removed,
  rejected_unknown_lifecycle,
  rejected_illegal_transition,
  rejected_capability_state,
  rejected_ambiguous_capability,
  rejected_generation_exhausted,
};

class ExtensionRegistry {
 public:
  RegistryResult register_extension(const ExtensionCandidate& candidate,
                                    const RegistryAssignment& assignment);
  RegistryResult unregister_extension(const std::string& package_id);
  RegistryResult transition(const std::string& package_id,
                            LifecycleState target,
                            FailureClass failure = FailureClass::none);
  RegistryResult validate_capabilities(
      const std::string& package_id,
      const std::vector<std::string>& validated_capabilities);
  RegistryResult activate_capabilities(
      const std::string& package_id,
      const std::vector<std::string>& active_capabilities);
  RegistryResult replace_package_version(
      const std::string& package_id, ContractVersion version,
      std::uint64_t expected_authorization_generation);

  const ExtensionRecord* find(const std::string& package_id) const;
  std::vector<CapabilityProvider> resolve(
      const std::string& capability) const;
  std::vector<CapabilityCandidate> inspect_capability(
      const std::string& capability) const;

 private:
  ExtensionRecord* find_mutable(const std::string& package_id);
  bool assign_new_authorization_generation(ExtensionRecord& record);
  std::vector<ExtensionRecord> records_;
  std::uint64_t next_authorization_generation_{1};
};

}  // namespace zie::extensions
