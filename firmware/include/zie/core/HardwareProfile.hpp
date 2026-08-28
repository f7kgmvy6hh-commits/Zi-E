#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "zie/core/HardwareRegistry.hpp"
#include "zie/extensions/ExtensionRegistry.hpp"

namespace zie::core {
enum class ProfileId { bench_minimal, stage1, full_prototype };
struct ProfileValidation {
  bool valid{false};
  std::vector<std::string> missing_required;
};
ProfileValidation validate_profile(ProfileId profile, const HardwareRegistry& hardware);
const char* profile_name(ProfileId profile);

enum class HardwareDomain {
  display,
  camera,
  audio_input,
  audio_output,
  range,
  proximity,
  cliff,
  imu,
  drive,
  arm,
  gripper,
  tool,
  rgb,
  touch,
  power,
  battery,
  network,
};

enum class ProfileRequirement { required, optional };
enum class ProfileCardinality {
  exactly_one,
  zero_or_one,
  one_or_more,
  zero_or_more,
};
enum class ProfileOwnership {
  registry_managed,
  protected_safety_core,
};
enum class ProfileSource { built_in, operator_approved };
enum class RobotPlatformClass { zie_desktop_companion };
enum class SemanticConstraintDomain {
  variant,
  placement,
  performance_class,
  tool_class,
};

struct SemanticHardwareConstraint {
  SemanticConstraintDomain domain{SemanticConstraintDomain::variant};
  std::string value;
};

struct HardwareProfileEntry {
  HardwareDomain domain{HardwareDomain::display};
  std::string logical_slot;
  std::string semantic_capability;
  extensions::ExtensionCategory compatible_category{
      extensions::ExtensionCategory::hardware_display};
  ProfileRequirement requirement{ProfileRequirement::required};
  ProfileCardinality cardinality{ProfileCardinality::exactly_one};
  ProfileOwnership ownership{ProfileOwnership::registry_managed};
  std::vector<SemanticHardwareConstraint> constraints;
};

struct HardwareProfileDefinition {
  std::string profile_id;
  extensions::ContractVersion schema_version{};
  std::uint64_t revision{0};
  RobotPlatformClass platform{RobotPlatformClass::zie_desktop_companion};
  std::vector<HardwareProfileEntry> entries;
};

// Supplied by the trusted profile catalog boundary, never derived from definition
// content or an extension/device claim.
struct HardwareProfileAuthority {
  std::string profile_id;
  std::string owner_id;
  ProfileSource source{ProfileSource::operator_approved};
};

enum class HardwareProfileIssue {
  invalid_profile_id,
  invalid_owner,
  identity_mismatch,
  unknown_source,
  unknown_schema,
  invalid_revision,
  unknown_platform,
  too_many_entries,
  unknown_domain,
  unknown_category,
  unknown_requirement,
  unknown_cardinality,
  unknown_ownership,
  unknown_constraint,
  invalid_slot,
  duplicate_slot,
  duplicate_capability,
  conflicting_requirement,
  invalid_capability,
  incompatible_category,
  impossible_cardinality,
  protected_ownership,
  raw_control_namespace,
  duplicate_constraint,
};

enum class HardwareProfileState {
  declared,
  validated,
  resolved,
  active,
  invalidated,
};

enum class ProfileEntryStatus {
  resolved,
  optional_missing,
  required_missing,
  ambiguous,
  wrong_profile,
  unsafe_capability,
  incompatible_category,
  incompatible_constraints,
  inactive,
  protected_ownership,
  revoked,
};

struct ResolvedHardwareDevice {
  std::string package_id;
  std::string logical_device_instance_id;
  std::string active_capability;
  std::uint64_t authorization_generation{0};
};

struct ProfileEntryResolution {
  std::string logical_slot;
  std::string semantic_capability;
  ProfileEntryStatus status{ProfileEntryStatus::required_missing};
  std::vector<ResolvedHardwareDevice> devices;
  std::string reason_package_id;
};

struct HardwareProfileIdentity {
  std::string profile_id;
  extensions::ContractVersion schema_version{};
  std::uint64_t revision{0};
  std::uint64_t generation{0};
  std::string owner_id;
  ProfileSource source{ProfileSource::operator_approved};
  RobotPlatformClass platform{RobotPlatformClass::zie_desktop_companion};
};

struct HardwareProfileResolution {
  HardwareProfileIdentity identity;
  bool ready{false};
  std::vector<ProfileEntryResolution> entries;
};

enum class HardwareProfileResult {
  declared,
  validated,
  resolved,
  activated,
  deactivated,
  invalidated,
  rejected_invalid_manager,
  rejected_not_found,
  rejected_duplicate_profile,
  rejected_identity,
  rejected_validation,
  rejected_lifecycle,
  rejected_unresolved,
  rejected_generation_exhausted,
};

constexpr extensions::ContractVersion supported_hardware_profile_schema() {
  return {1, 0, 0};
}

class HardwareProfileManager {
 public:
  HardwareProfileManager(const extensions::ExtensionRegistry& registry,
                         std::size_t max_profiles, std::size_t max_entries,
                         std::size_t max_matches_per_entry)
      : registry_(registry),
        max_profiles_(max_profiles),
        max_entries_(max_entries),
        max_matches_per_entry_(max_matches_per_entry) {}

  HardwareProfileResult declare_profile(
      const HardwareProfileDefinition& definition,
      const HardwareProfileAuthority& authority);
  HardwareProfileResult validate_profile(const std::string& profile_id);
  HardwareProfileResult resolve_profile(const std::string& profile_id);
  HardwareProfileResult activate_profile(const std::string& profile_id);
  HardwareProfileResult deactivate_profile(const std::string& profile_id);

  std::optional<HardwareProfileResolution> resolution(
      const std::string& profile_id);
  std::vector<HardwareProfileIssue> validation_issues(
      const std::string& profile_id) const;
  std::optional<HardwareProfileResolution> active_profile();
  bool active_profile_has_protected_safety();
  HardwareProfileState state(const std::string& profile_id);

 private:
  struct Record {
    HardwareProfileDefinition definition;
    HardwareProfileAuthority authority;
    HardwareProfileState state{HardwareProfileState::declared};
    std::vector<HardwareProfileIssue> validation_issues;
    std::optional<HardwareProfileResolution> resolution;
  };
  Record* find_mutable(const std::string& profile_id);
  const Record* find(const std::string& profile_id) const;
  bool resolution_is_current(Record& record);
  bool next_generation(std::uint64_t& generation);

  const extensions::ExtensionRegistry& registry_;
  std::size_t max_profiles_{0};
  std::size_t max_entries_{0};
  std::size_t max_matches_per_entry_{0};
  std::uint64_t next_generation_{1};
  std::vector<Record> records_;
  std::string active_profile_id_;
};
}  // namespace zie::core
