#include "zie/extensions/TransactionalConfiguration.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <system_error>

namespace zie::extensions {
namespace {

bool known(const ConfigurationValueDomain value) {
  switch (value) {
    case ConfigurationValueDomain::semantic_text:
    case ConfigurationValueDomain::bounded_integer:
    case ConfigurationValueDomain::boolean:
      return true;
  }
  return false;
}

bool known(const ConfigurationEffect value) {
  switch (value) {
    case ConfigurationEffect::semantic_behavior:
    case ConfigurationEffect::semantic_hardware:
      return true;
  }
  return false;
}

bool valid_key(const std::string& value) {
  if (value.empty() || !std::isalnum(static_cast<unsigned char>(value.front()))) {
    return false;
  }
  return std::all_of(value.begin(), value.end(), [](const unsigned char c) {
    return std::islower(c) || std::isdigit(c) || c == '.' || c == '_' ||
           c == '-';
  });
}

bool begins_with(const std::string& value, const std::string& prefix) {
  return value.compare(0, prefix.size(), prefix) == 0;
}

bool authority_or_raw_key(const std::string& key) {
  static const std::vector<std::string> forbidden{
      "trust.",       "permissions.", "capabilities.", "controller.",
      "safety.",      "safety_bypass.", "gpio.",       "pwm.",
      "register.",    "actuator.",    "driver.",       "can."};
  return std::any_of(forbidden.begin(), forbidden.end(),
                     [&key](const std::string& prefix) {
                       return begins_with(key, prefix);
                     });
}

template <typename T, typename Key>
bool duplicates(const std::vector<T>& values, Key key) {
  for (std::size_t left = 0; left < values.size(); ++left) {
    for (std::size_t right = left + 1; right < values.size(); ++right) {
      if (key(values[left]) == key(values[right])) return true;
    }
  }
  return false;
}

bool same_binding(const ConfigurationBinding& left,
                  const ConfigurationBinding& right) {
  return left.package_id == right.package_id &&
         left.logical_device_instance_id == right.logical_device_instance_id &&
         left.hardware_profile_id == right.hardware_profile_id;
}

ConfigurationBinding binding_for(const ExtensionRecord& record) {
  return {record.manifest.id, record.device_identity.logical.instance_id,
          record.device_identity.hardware_profile.profile_id};
}

bool may_commit(const LifecycleState state) {
  return state == LifecycleState::inactive || state == LifecycleState::active ||
         state == LifecycleState::degraded;
}

bool valid_value(const ConfigurationValue& value,
                 const ConfigurationDeclaration& declaration) {
  switch (value.domain) {
    case ConfigurationValueDomain::semantic_text:
      return !value.value.empty();
    case ConfigurationValueDomain::boolean:
      return value.value == "true" || value.value == "false";
    case ConfigurationValueDomain::bounded_integer: {
      if (!declaration.minimum.has_value() ||
          !declaration.maximum.has_value() || value.value.empty()) {
        return false;
      }
      std::int64_t parsed{0};
      const char* const begin = value.value.data();
      const char* const end = begin + value.value.size();
      const auto result = std::from_chars(begin, end, parsed, 10);
      return result.ec == std::errc{} && result.ptr == end &&
             parsed >= *declaration.minimum &&
             parsed <= *declaration.maximum;
    }
  }
  return false;
}

}  // namespace

ConfigurationResult TransactionalConfiguration::declare_configuration(
    const std::string& package_id,
    const std::vector<ConfigurationDeclaration>& declarations) {
  const auto* extension = registry_.find(package_id);
  if (extension == nullptr) return ConfigurationResult::rejected_not_found;
  if (find(package_id) != nullptr) {
    return ConfigurationResult::rejected_duplicate_declaration;
  }
  if (duplicates(declarations,
                 [](const ConfigurationDeclaration& value) {
                   return value.key;
                 })) {
    return ConfigurationResult::rejected_duplicate_key;
  }
  for (const auto& declaration : declarations) {
    if (!known(declaration.domain) || !known(declaration.effect)) {
      return ConfigurationResult::rejected_unknown_domain;
    }
    if (!valid_key(declaration.key)) {
      return ConfigurationResult::rejected_invalid_declaration;
    }
    if (authority_or_raw_key(declaration.key)) {
      return ConfigurationResult::rejected_authority_escalation;
    }
    if (declaration.domain == ConfigurationValueDomain::bounded_integer) {
      if (!declaration.minimum.has_value() ||
          !declaration.maximum.has_value() ||
          *declaration.minimum > *declaration.maximum) {
        return ConfigurationResult::rejected_invalid_declaration;
      }
    } else if (declaration.minimum.has_value() ||
               declaration.maximum.has_value()) {
      return ConfigurationResult::rejected_invalid_declaration;
    }
  }
  ConfigurationRecord record;
  record.authoritative_binding = binding_for(*extension);
  record.declared_configuration = declarations;
  records_.push_back(std::move(record));
  return ConfigurationResult::declared;
}

ConfigurationResult TransactionalConfiguration::stage(
    const ConfigurationCandidate& candidate,
    const ConfigurationValidationContext& context) {
  auto* record = find_mutable(candidate.binding.package_id);
  if (record == nullptr) return ConfigurationResult::rejected_not_found;
  const auto* extension = registry_.find(candidate.binding.package_id);
  if (extension == nullptr ||
      !same_binding(candidate.binding, context.authoritative_requester) ||
      !same_binding(candidate.binding, record->authoritative_binding) ||
      !same_binding(candidate.binding, binding_for(*extension))) {
    return ConfigurationResult::rejected_identity_mismatch;
  }
  if (candidate.revision == 0 ||
      (record->active_configuration.has_value() &&
       candidate.revision <= record->active_configuration->revision) ||
      (record->staged_candidate.has_value() &&
       candidate.revision <= record->staged_candidate->revision)) {
    return ConfigurationResult::rejected_stale_revision;
  }
  record->staged_candidate = ConfigurationSnapshot{
      candidate.binding, candidate.revision, 0, candidate.values};
  record->validated_configuration.reset();
  return ConfigurationResult::staged;
}

ConfigurationResult TransactionalConfiguration::validate_staged(
    const std::string& package_id) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return ConfigurationResult::rejected_not_found;
  if (!record->staged_candidate.has_value()) {
    return ConfigurationResult::rejected_no_staged_candidate;
  }
  const auto& candidate = *record->staged_candidate;
  if (duplicates(candidate.values, [](const ConfigurationValue& value) {
        return value.key;
      })) {
    return ConfigurationResult::rejected_duplicate_key;
  }
  for (const auto& value : candidate.values) {
    if (!known(value.domain)) {
      return ConfigurationResult::rejected_unknown_domain;
    }
    if (authority_or_raw_key(value.key)) {
      return ConfigurationResult::rejected_authority_escalation;
    }
    const auto declaration = std::find_if(
        record->declared_configuration.begin(),
        record->declared_configuration.end(),
        [&value](const ConfigurationDeclaration& declared) {
          return declared.key == value.key;
        });
    if (declaration == record->declared_configuration.end()) {
      return ConfigurationResult::rejected_undeclared_key;
    }
    if (declaration->domain != value.domain ||
        !valid_value(value, *declaration)) {
      return ConfigurationResult::rejected_invalid_value;
    }
  }
  record->validated_configuration = candidate;
  return ConfigurationResult::validated;
}

ConfigurationResult TransactionalConfiguration::commit(
    const std::string& package_id) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return ConfigurationResult::rejected_not_found;
  const auto* extension = registry_.find(package_id);
  if (extension == nullptr || !may_commit(extension->lifecycle)) {
    return ConfigurationResult::rejected_lifecycle;
  }
  if (!record->validated_configuration.has_value() ||
      !record->staged_candidate.has_value() ||
      record->validated_configuration->revision !=
          record->staged_candidate->revision) {
    return ConfigurationResult::rejected_not_validated;
  }
  if (record->active_configuration.has_value() &&
      record->validated_configuration->revision <=
          record->active_configuration->revision) {
    return ConfigurationResult::rejected_stale_revision;
  }

  ConfigurationSnapshot committed = *record->validated_configuration;
  committed.generation = record->active_configuration.has_value()
                             ? record->active_configuration->generation + 1
                             : 1;
  record->active_configuration = std::move(committed);
  record->staged_candidate.reset();
  record->validated_configuration.reset();
  return ConfigurationResult::committed;
}

ConfigurationResult TransactionalConfiguration::rollback(
    const std::string& package_id) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return ConfigurationResult::rejected_not_found;
  record->staged_candidate.reset();
  record->validated_configuration.reset();
  return ConfigurationResult::rolled_back;
}

const ConfigurationRecord* TransactionalConfiguration::find(
    const std::string& package_id) const {
  const auto found = std::find_if(
      records_.begin(), records_.end(),
      [&package_id](const ConfigurationRecord& record) {
        return record.authoritative_binding.package_id == package_id;
      });
  return found == records_.end() ? nullptr : &*found;
}

ConfigurationRecord* TransactionalConfiguration::find_mutable(
    const std::string& package_id) {
  const auto found = std::find_if(
      records_.begin(), records_.end(),
      [&package_id](const ConfigurationRecord& record) {
        return record.authoritative_binding.package_id == package_id;
      });
  return found == records_.end() ? nullptr : &*found;
}

}  // namespace zie::extensions
