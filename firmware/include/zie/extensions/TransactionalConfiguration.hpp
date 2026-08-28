#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "zie/extensions/ExtensionRegistry.hpp"

namespace zie::extensions {

enum class ConfigurationValueDomain {
  semantic_text,
  bounded_integer,
  boolean,
};

enum class ConfigurationEffect {
  semantic_behavior,
  semantic_hardware,
};

struct ConfigurationDeclaration {
  std::string key;
  ConfigurationValueDomain domain{ConfigurationValueDomain::semantic_text};
  ConfigurationEffect effect{ConfigurationEffect::semantic_behavior};
  std::optional<std::int64_t> minimum;
  std::optional<std::int64_t> maximum;
};

struct ConfigurationValue {
  std::string key;
  ConfigurationValueDomain domain{ConfigurationValueDomain::semantic_text};
  std::string value;
};

struct ConfigurationBinding {
  std::string package_id;
  std::string logical_device_instance_id;
  std::string hardware_profile_id;
};

struct ConfigurationCandidate {
  ConfigurationBinding binding;
  std::uint64_t revision{0};
  std::vector<ConfigurationValue> values;
};

// Supplied by the trusted registry/call boundary, never derived from candidate data.
struct ConfigurationValidationContext {
  ConfigurationBinding authoritative_requester;
};

struct ConfigurationSnapshot {
  ConfigurationBinding binding;
  std::uint64_t revision{0};
  std::uint64_t generation{0};
  std::vector<ConfigurationValue> values;
};

struct ConfigurationRecord {
  ConfigurationBinding authoritative_binding;
  std::vector<ConfigurationDeclaration> declared_configuration;
  std::optional<ConfigurationSnapshot> staged_candidate;
  std::optional<ConfigurationSnapshot> validated_configuration;
  std::optional<ConfigurationSnapshot> active_configuration;
};

enum class ConfigurationResult {
  declared,
  staged,
  validated,
  committed,
  rolled_back,
  rejected_not_found,
  rejected_duplicate_declaration,
  rejected_invalid_declaration,
  rejected_no_staged_candidate,
  rejected_not_validated,
  rejected_stale_revision,
  rejected_identity_mismatch,
  rejected_unknown_domain,
  rejected_undeclared_key,
  rejected_duplicate_key,
  rejected_invalid_value,
  rejected_authority_escalation,
  rejected_lifecycle,
};

class TransactionalConfiguration {
 public:
  explicit TransactionalConfiguration(const ExtensionRegistry& registry)
      : registry_(registry) {}

  ConfigurationResult declare_configuration(
      const std::string& package_id,
      const std::vector<ConfigurationDeclaration>& declarations);
  ConfigurationResult stage(
      const ConfigurationCandidate& candidate,
      const ConfigurationValidationContext& context);
  ConfigurationResult validate_staged(const std::string& package_id);
  ConfigurationResult commit(const std::string& package_id);
  ConfigurationResult rollback(const std::string& package_id);

  const ConfigurationRecord* find(const std::string& package_id) const;
  bool active_authorized(const std::string& package_id) const;

 private:
  ConfigurationRecord* find_mutable(const std::string& package_id);
  const ExtensionRegistry& registry_;
  std::vector<ConfigurationRecord> records_;
};

}  // namespace zie::extensions
