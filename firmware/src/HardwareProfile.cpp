#include "zie/core/HardwareProfile.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <limits>

namespace zie::core {
namespace {

void require(const bool present, const char* name, ProfileValidation& result) {
  if (!present) result.missing_required.emplace_back(name);
}

void require_owner(const bool present, const ControllerOwner actual,
                   const ControllerOwner expected, const char* name,
                   ProfileValidation& result) {
  if (present && actual != expected) {
    result.missing_required.emplace_back(std::string(name) + " owner");
  }
}

template <typename T>
bool contains(const std::vector<T>& values, const T& value) {
  return std::find(values.begin(), values.end(), value) != values.end();
}

void add_issue(std::vector<HardwareProfileIssue>& issues,
               const HardwareProfileIssue issue) {
  if (!contains(issues, issue)) issues.push_back(issue);
}

bool valid_key(const std::string& value) {
  if (value.empty() ||
      !std::isalnum(static_cast<unsigned char>(value.front()))) {
    return false;
  }
  return std::all_of(value.begin(), value.end(), [](const unsigned char value) {
    return std::islower(value) || std::isdigit(value) || value == '.' ||
           value == '_' || value == '-';
  });
}

bool begins_with(const std::string& value, const std::string& prefix) {
  return value.compare(0, prefix.size(), prefix) == 0;
}

bool contains_namespace_token(const std::string& value,
                              const std::string& token) {
  std::size_t offset = 0;
  while (offset <= value.size()) {
    const auto end = value.find_first_of("._-", offset);
    const auto size = end == std::string::npos ? value.size() - offset
                                               : end - offset;
    if (value.compare(offset, size, token) == 0 && size == token.size()) {
      return true;
    }
    if (end == std::string::npos) break;
    offset = end + 1;
  }
  return false;
}

bool raw_namespace(const std::string& value) {
  static const std::vector<std::string> forbidden{
      "raw",      "gpio",     "pwm",      "register", "motor",
      "actuator", "setpoint", "safety",   "safety_bypass",
      "can",      "usb",      "com",      "ip",       "discovery"};
  return std::any_of(forbidden.begin(), forbidden.end(),
                     [&value](const std::string& token) {
                       return contains_namespace_token(value, token);
                     });
}

bool known(const HardwareDomain value) {
  switch (value) {
    case HardwareDomain::display:
    case HardwareDomain::camera:
    case HardwareDomain::audio_input:
    case HardwareDomain::audio_output:
    case HardwareDomain::range:
    case HardwareDomain::proximity:
    case HardwareDomain::cliff:
    case HardwareDomain::imu:
    case HardwareDomain::drive:
    case HardwareDomain::arm:
    case HardwareDomain::gripper:
    case HardwareDomain::tool:
    case HardwareDomain::rgb:
    case HardwareDomain::touch:
    case HardwareDomain::power:
    case HardwareDomain::battery:
    case HardwareDomain::network:
      return true;
  }
  return false;
}

bool known(const ProfileRequirement value) {
  switch (value) {
    case ProfileRequirement::required:
    case ProfileRequirement::optional:
      return true;
  }
  return false;
}

bool known(const ProfileCardinality value) {
  switch (value) {
    case ProfileCardinality::exactly_one:
    case ProfileCardinality::zero_or_one:
    case ProfileCardinality::one_or_more:
    case ProfileCardinality::zero_or_more:
      return true;
  }
  return false;
}

bool known(const ProfileOwnership value) {
  switch (value) {
    case ProfileOwnership::registry_managed:
    case ProfileOwnership::protected_safety_core:
      return true;
  }
  return false;
}

bool known(const ProfileSource value) {
  switch (value) {
    case ProfileSource::built_in:
    case ProfileSource::operator_approved:
      return true;
  }
  return false;
}

bool known(const RobotPlatformClass value) {
  switch (value) {
    case RobotPlatformClass::zie_desktop_companion:
      return true;
  }
  return false;
}

bool known(const SemanticConstraintDomain value) {
  switch (value) {
    case SemanticConstraintDomain::variant:
    case SemanticConstraintDomain::placement:
    case SemanticConstraintDomain::performance_class:
    case SemanticConstraintDomain::tool_class:
      return true;
  }
  return false;
}

bool known(const extensions::ExtensionCategory value) {
  switch (value) {
    case extensions::ExtensionCategory::hardware_display:
    case extensions::ExtensionCategory::hardware_camera:
    case extensions::ExtensionCategory::hardware_audio_input:
    case extensions::ExtensionCategory::hardware_audio_output:
    case extensions::ExtensionCategory::hardware_range:
    case extensions::ExtensionCategory::hardware_proximity:
    case extensions::ExtensionCategory::hardware_cliff:
    case extensions::ExtensionCategory::hardware_imu:
    case extensions::ExtensionCategory::hardware_drive:
    case extensions::ExtensionCategory::hardware_motor_driver:
    case extensions::ExtensionCategory::hardware_actuator:
    case extensions::ExtensionCategory::hardware_arm:
    case extensions::ExtensionCategory::hardware_gripper:
    case extensions::ExtensionCategory::hardware_tool:
    case extensions::ExtensionCategory::hardware_power:
    case extensions::ExtensionCategory::hardware_battery:
    case extensions::ExtensionCategory::hardware_rgb:
    case extensions::ExtensionCategory::hardware_touch:
    case extensions::ExtensionCategory::hardware_network:
    case extensions::ExtensionCategory::behavior:
    case extensions::ExtensionCategory::ai_provider:
    case extensions::ExtensionCategory::ai_tool:
    case extensions::ExtensionCategory::voice_stt:
    case extensions::ExtensionCategory::voice_tts:
    case extensions::ExtensionCategory::voice_wakeword:
    case extensions::ExtensionCategory::face_pack:
    case extensions::ExtensionCategory::emotion_pack:
    case extensions::ExtensionCategory::rgb_pack:
    case extensions::ExtensionCategory::sound_pack:
    case extensions::ExtensionCategory::automation:
    case extensions::ExtensionCategory::integration:
      return true;
  }
  return false;
}

const char* base_capability(const HardwareDomain domain) {
  switch (domain) {
    case HardwareDomain::display:
      return "hardware.display";
    case HardwareDomain::camera:
      return "hardware.camera";
    case HardwareDomain::audio_input:
      return "hardware.audio-input";
    case HardwareDomain::audio_output:
      return "hardware.audio-output";
    case HardwareDomain::range:
      return "hardware.range";
    case HardwareDomain::proximity:
      return "hardware.proximity";
    case HardwareDomain::cliff:
      return "hardware.cliff";
    case HardwareDomain::imu:
      return "hardware.imu";
    case HardwareDomain::drive:
      return "hardware.drive";
    case HardwareDomain::arm:
      return "hardware.arm";
    case HardwareDomain::gripper:
      return "hardware.gripper";
    case HardwareDomain::tool:
      return "hardware.tool";
    case HardwareDomain::rgb:
      return "hardware.rgb";
    case HardwareDomain::touch:
      return "hardware.touch";
    case HardwareDomain::power:
      return "hardware.power";
    case HardwareDomain::battery:
      return "hardware.battery";
    case HardwareDomain::network:
      return "hardware.network";
  }
  return nullptr;
}

extensions::ExtensionCategory category_for(const HardwareDomain domain) {
  switch (domain) {
    case HardwareDomain::display:
      return extensions::ExtensionCategory::hardware_display;
    case HardwareDomain::camera:
      return extensions::ExtensionCategory::hardware_camera;
    case HardwareDomain::audio_input:
      return extensions::ExtensionCategory::hardware_audio_input;
    case HardwareDomain::audio_output:
      return extensions::ExtensionCategory::hardware_audio_output;
    case HardwareDomain::range:
      return extensions::ExtensionCategory::hardware_range;
    case HardwareDomain::proximity:
      return extensions::ExtensionCategory::hardware_proximity;
    case HardwareDomain::cliff:
      return extensions::ExtensionCategory::hardware_cliff;
    case HardwareDomain::imu:
      return extensions::ExtensionCategory::hardware_imu;
    case HardwareDomain::drive:
      return extensions::ExtensionCategory::hardware_drive;
    case HardwareDomain::arm:
      return extensions::ExtensionCategory::hardware_arm;
    case HardwareDomain::gripper:
      return extensions::ExtensionCategory::hardware_gripper;
    case HardwareDomain::tool:
      return extensions::ExtensionCategory::hardware_tool;
    case HardwareDomain::rgb:
      return extensions::ExtensionCategory::hardware_rgb;
    case HardwareDomain::touch:
      return extensions::ExtensionCategory::hardware_touch;
    case HardwareDomain::power:
      return extensions::ExtensionCategory::hardware_power;
    case HardwareDomain::battery:
      return extensions::ExtensionCategory::hardware_battery;
    case HardwareDomain::network:
      return extensions::ExtensionCategory::hardware_network;
  }
  return extensions::ExtensionCategory::behavior;
}

bool capability_matches_domain(const std::string& capability,
                               const HardwareDomain domain) {
  const char* const base = base_capability(domain);
  if (base == nullptr) return false;
  if (capability == base) return true;
  const std::string prefix = std::string(base) + ".";
  return begins_with(capability, prefix) &&
         valid_key(capability.substr(prefix.size()));
}

bool protected_domain(const HardwareDomain domain) {
  switch (domain) {
    case HardwareDomain::cliff:
    case HardwareDomain::drive:
    case HardwareDomain::power:
    case HardwareDomain::battery:
      return true;
    case HardwareDomain::display:
    case HardwareDomain::camera:
    case HardwareDomain::audio_input:
    case HardwareDomain::audio_output:
    case HardwareDomain::range:
    case HardwareDomain::proximity:
    case HardwareDomain::imu:
    case HardwareDomain::arm:
    case HardwareDomain::gripper:
    case HardwareDomain::tool:
    case HardwareDomain::rgb:
    case HardwareDomain::touch:
    case HardwareDomain::network:
      return false;
  }
  return false;
}

bool single_cardinality(const ProfileCardinality cardinality) {
  return cardinality == ProfileCardinality::exactly_one ||
         cardinality == ProfileCardinality::zero_or_one;
}

bool zero_allowed(const ProfileCardinality cardinality) {
  return cardinality == ProfileCardinality::zero_or_one ||
         cardinality == ProfileCardinality::zero_or_more;
}

bool same_version(const extensions::ContractVersion left,
                  const extensions::ContractVersion right) {
  return left.major == right.major && left.minor == right.minor &&
         left.patch == right.patch;
}

bool active_candidate(const extensions::CapabilityCandidate& candidate) {
  return candidate.lifecycle == extensions::LifecycleState::active &&
         candidate.capability_validated && candidate.capability_active;
}

extensions::SemanticAttributeDomain attribute_domain(
    const SemanticConstraintDomain domain) {
  switch (domain) {
    case SemanticConstraintDomain::variant:
      return extensions::SemanticAttributeDomain::variant;
    case SemanticConstraintDomain::placement:
      return extensions::SemanticAttributeDomain::placement;
    case SemanticConstraintDomain::performance_class:
      return extensions::SemanticAttributeDomain::performance_class;
    case SemanticConstraintDomain::tool_class:
      return extensions::SemanticAttributeDomain::tool_class;
  }
  return static_cast<extensions::SemanticAttributeDomain>(-1);
}

bool constraints_match(const extensions::CapabilityCandidate& candidate,
                       const HardwareProfileEntry& entry) {
  return std::all_of(
      entry.constraints.begin(), entry.constraints.end(),
      [&candidate](const SemanticHardwareConstraint& constraint) {
        return std::any_of(
            candidate.semantic_attributes.begin(),
            candidate.semantic_attributes.end(),
            [&constraint](const extensions::SemanticDeviceAttribute& attribute) {
              return attribute.domain == attribute_domain(constraint.domain) &&
                     attribute.value == constraint.value;
            });
      });
}

}  // namespace

const char* profile_name(const ProfileId profile) {
  switch (profile) {
    case ProfileId::bench_minimal:
      return "bench-minimal";
    case ProfileId::stage1:
      return "stage1";
    case ProfileId::full_prototype:
      return "full-prototype";
  }
  return "unknown";
}

ProfileValidation validate_profile(const ProfileId profile,
                                   const HardwareRegistry& hardware) {
  ProfileValidation result;
  require(hardware.display, "Display", result);
  require(hardware.motion, "MotionController", result);
  require(hardware.battery, "Battery", result);
  require(hardware.range_sensor, "RangeSensor", result);
  require(hardware.audio, "Audio", result);
  const auto multimedia_owner = profile == ProfileId::bench_minimal
                                    ? ControllerOwner::host_bench
                                    : ControllerOwner::esp32_s3;
  const auto safety_owner = profile == ProfileId::bench_minimal
                                ? ControllerOwner::host_bench
                                : ControllerOwner::stm32;
  require_owner(hardware.display, hardware.display_owner, multimedia_owner,
                "Display", result);
  require_owner(hardware.audio, hardware.audio_owner, multimedia_owner,
                "Audio", result);
  require_owner(hardware.motion, hardware.motion_owner, safety_owner,
                "MotionController", result);
  require_owner(hardware.battery, hardware.battery_owner, safety_owner,
                "Battery", result);
  require_owner(hardware.camera, hardware.camera_owner, multimedia_owner,
                "Camera", result);
  require_owner(hardware.belly_light_matrix,
                hardware.belly_light_matrix_owner, multimedia_owner,
                "BellyLightMatrix", result);
  require_owner(hardware.head_motion, hardware.head_motion_owner, safety_owner,
                "HeadMotion", result);
  require_owner(hardware.arms, hardware.arms_owner, safety_owner, "Arms",
                result);
  require_owner(hardware.safety_sensors, hardware.safety_sensors_owner,
                safety_owner, "SafetySensors", result);
  require_owner(hardware.range_sensor, hardware.range_sensor_owner,
                safety_owner, "RangeSensor", result);
  if (profile != ProfileId::bench_minimal) {
    require(hardware.safety_sensors, "SafetySensors", result);
  }
  if (profile == ProfileId::full_prototype) {
    require(hardware.camera, "Camera", result);
    require(hardware.belly_light_matrix, "BellyLightMatrix", result);
    require(hardware.head_motion, "HeadMotion", result);
    require(hardware.arms, "Arms", result);
  }
  result.valid = result.missing_required.empty();
  return result;
}

HardwareProfileManager::Record* HardwareProfileManager::find_mutable(
    const std::string& profile_id) {
  const auto found = std::find_if(records_.begin(), records_.end(),
                                  [&profile_id](const Record& record) {
                                    return record.definition.profile_id ==
                                           profile_id;
                                  });
  return found == records_.end() ? nullptr : &*found;
}

const HardwareProfileManager::Record* HardwareProfileManager::find(
    const std::string& profile_id) const {
  const auto found = std::find_if(records_.begin(), records_.end(),
                                  [&profile_id](const Record& record) {
                                    return record.definition.profile_id ==
                                           profile_id;
                                  });
  return found == records_.end() ? nullptr : &*found;
}

bool HardwareProfileManager::next_generation(std::uint64_t& generation) {
  if (next_generation_ == 0 ||
      next_generation_ == std::numeric_limits<std::uint64_t>::max()) {
    return false;
  }
  generation = next_generation_++;
  return true;
}

HardwareProfileResult HardwareProfileManager::declare_profile(
    const HardwareProfileDefinition& definition,
    const HardwareProfileAuthority& authority) {
  if (max_profiles_ == 0 || max_entries_ == 0 ||
      max_matches_per_entry_ == 0) {
    return HardwareProfileResult::rejected_invalid_manager;
  }
  if (records_.size() >= max_profiles_ ||
      find(definition.profile_id) != nullptr) {
    return HardwareProfileResult::rejected_duplicate_profile;
  }
  if (!valid_key(definition.profile_id) || !valid_key(authority.profile_id) ||
      !valid_key(authority.owner_id) || !known(authority.source) ||
      definition.profile_id != authority.profile_id) {
    return HardwareProfileResult::rejected_identity;
  }
  records_.push_back({definition, authority, HardwareProfileState::declared,
                      {}, std::nullopt});
  return HardwareProfileResult::declared;
}

HardwareProfileResult HardwareProfileManager::validate_profile(
    const std::string& profile_id) {
  auto* record = find_mutable(profile_id);
  if (record == nullptr) return HardwareProfileResult::rejected_not_found;
  if (record->state != HardwareProfileState::declared) {
    return HardwareProfileResult::rejected_lifecycle;
  }
  auto& issues = record->validation_issues;
  issues.clear();
  const auto& definition = record->definition;
  if (!valid_key(definition.profile_id)) {
    add_issue(issues, HardwareProfileIssue::invalid_profile_id);
  }
  if (!valid_key(record->authority.owner_id)) {
    add_issue(issues, HardwareProfileIssue::invalid_owner);
  }
  if (definition.profile_id != record->authority.profile_id) {
    add_issue(issues, HardwareProfileIssue::identity_mismatch);
  }
  if (!known(record->authority.source)) {
    add_issue(issues, HardwareProfileIssue::unknown_source);
  }
  if (!same_version(definition.schema_version,
                    supported_hardware_profile_schema())) {
    add_issue(issues, HardwareProfileIssue::unknown_schema);
  }
  if (definition.revision == 0) {
    add_issue(issues, HardwareProfileIssue::invalid_revision);
  }
  if (!known(definition.platform)) {
    add_issue(issues, HardwareProfileIssue::unknown_platform);
  }
  if (definition.entries.size() > max_entries_) {
    add_issue(issues, HardwareProfileIssue::too_many_entries);
  }
  for (std::size_t index = 0; index < definition.entries.size(); ++index) {
    const auto& entry = definition.entries[index];
    if (!known(entry.domain)) add_issue(issues, HardwareProfileIssue::unknown_domain);
    if (!known(entry.compatible_category)) {
      add_issue(issues, HardwareProfileIssue::unknown_category);
    }
    if (!known(entry.requirement)) {
      add_issue(issues, HardwareProfileIssue::unknown_requirement);
    }
    if (!known(entry.cardinality)) {
      add_issue(issues, HardwareProfileIssue::unknown_cardinality);
    }
    if (!known(entry.ownership)) {
      add_issue(issues, HardwareProfileIssue::unknown_ownership);
    }
    if (!valid_key(entry.logical_slot)) {
      add_issue(issues, HardwareProfileIssue::invalid_slot);
    }
    if (raw_namespace(entry.logical_slot) ||
        raw_namespace(entry.semantic_capability)) {
      add_issue(issues, HardwareProfileIssue::raw_control_namespace);
    }
    if (!valid_key(entry.semantic_capability) ||
        !known(entry.domain) ||
        !capability_matches_domain(entry.semantic_capability, entry.domain)) {
      add_issue(issues, HardwareProfileIssue::invalid_capability);
    }
    if (known(entry.domain) && known(entry.compatible_category) &&
        entry.compatible_category != category_for(entry.domain)) {
      add_issue(issues, HardwareProfileIssue::incompatible_category);
    }
    if (known(entry.requirement) && known(entry.cardinality) &&
        entry.requirement == ProfileRequirement::required &&
        zero_allowed(entry.cardinality)) {
      add_issue(issues, HardwareProfileIssue::impossible_cardinality);
    }
    if (known(entry.ownership) && known(entry.domain) &&
        entry.ownership == ProfileOwnership::protected_safety_core &&
        !protected_domain(entry.domain)) {
      add_issue(issues, HardwareProfileIssue::protected_ownership);
    }
    if (known(entry.ownership) && known(entry.domain) &&
        entry.ownership != ProfileOwnership::protected_safety_core &&
        protected_domain(entry.domain)) {
      add_issue(issues, HardwareProfileIssue::protected_ownership);
    }
    for (std::size_t constraint_index = 0;
         constraint_index < entry.constraints.size(); ++constraint_index) {
      const auto& constraint = entry.constraints[constraint_index];
      if (!known(constraint.domain) || !valid_key(constraint.value)) {
        add_issue(issues, HardwareProfileIssue::unknown_constraint);
      }
      if (raw_namespace(constraint.value)) {
        add_issue(issues, HardwareProfileIssue::raw_control_namespace);
      }
      for (std::size_t other = constraint_index + 1;
           other < entry.constraints.size(); ++other) {
        if (entry.constraints[other].domain == constraint.domain) {
          add_issue(issues, HardwareProfileIssue::duplicate_constraint);
        }
      }
    }
    for (std::size_t other = index + 1; other < definition.entries.size();
         ++other) {
      if (definition.entries[other].logical_slot == entry.logical_slot) {
        add_issue(issues, HardwareProfileIssue::duplicate_slot);
      }
      if (definition.entries[other].semantic_capability ==
          entry.semantic_capability) {
        add_issue(issues, HardwareProfileIssue::duplicate_capability);
        if (definition.entries[other].requirement != entry.requirement ||
            definition.entries[other].cardinality != entry.cardinality ||
            definition.entries[other].ownership != entry.ownership) {
          add_issue(issues, HardwareProfileIssue::conflicting_requirement);
        }
      }
    }
  }
  if (!issues.empty()) return HardwareProfileResult::rejected_validation;
  record->state = HardwareProfileState::validated;
  return HardwareProfileResult::validated;
}

HardwareProfileResult HardwareProfileManager::resolve_profile(
    const std::string& profile_id) {
  auto* record = find_mutable(profile_id);
  if (record == nullptr) return HardwareProfileResult::rejected_not_found;
  if (record->state != HardwareProfileState::validated &&
      record->state != HardwareProfileState::resolved &&
      record->state != HardwareProfileState::invalidated) {
    return HardwareProfileResult::rejected_lifecycle;
  }
  std::uint64_t generation{0};
  if (!next_generation(generation)) {
    return HardwareProfileResult::rejected_generation_exhausted;
  }
  HardwareProfileResolution resolution;
  resolution.identity = {record->definition.profile_id,
                         record->definition.schema_version,
                         record->definition.revision,
                         generation,
                         record->authority.owner_id,
                         record->authority.source,
                         record->definition.platform};
  resolution.ready = true;
  for (const auto& entry : record->definition.entries) {
    ProfileEntryResolution entry_result;
    entry_result.logical_slot = entry.logical_slot;
    entry_result.semantic_capability = entry.semantic_capability;
    auto candidates = registry_.inspect_capability(entry.semantic_capability);
    std::sort(candidates.begin(), candidates.end(),
              [](const extensions::CapabilityCandidate& left,
                 const extensions::CapabilityCandidate& right) {
                if (left.package_id != right.package_id) {
                  return left.package_id < right.package_id;
                }
                return left.logical_device_instance_id <
                       right.logical_device_instance_id;
              });
    std::vector<extensions::CapabilityCandidate> profile_candidates;
    std::copy_if(candidates.begin(), candidates.end(),
                 std::back_inserter(profile_candidates),
                 [&record](const extensions::CapabilityCandidate& candidate) {
                   return candidate.hardware_profile_id ==
                          record->definition.profile_id;
                 });
    std::vector<extensions::CapabilityCandidate> capability_candidates;
    std::copy_if(profile_candidates.begin(), profile_candidates.end(),
                 std::back_inserter(capability_candidates),
                 [&entry](const extensions::CapabilityCandidate& candidate) {
                   return capability_matches_domain(candidate.capability,
                                                    entry.domain) &&
                          !raw_namespace(candidate.capability);
                 });
    std::vector<extensions::CapabilityCandidate> category_candidates;
    std::copy_if(capability_candidates.begin(), capability_candidates.end(),
                 std::back_inserter(category_candidates),
                 [&entry](const extensions::CapabilityCandidate& candidate) {
                   return candidate.category == entry.compatible_category;
                 });
    std::vector<extensions::CapabilityCandidate> live_candidates;
    std::vector<extensions::CapabilityCandidate> constraint_candidates;
    std::copy_if(category_candidates.begin(), category_candidates.end(),
                 std::back_inserter(constraint_candidates),
                 [&entry](const extensions::CapabilityCandidate& candidate) {
                   return constraints_match(candidate, entry);
                 });
    std::copy_if(constraint_candidates.begin(), constraint_candidates.end(),
                 std::back_inserter(live_candidates), active_candidate);
    std::vector<extensions::CapabilityCandidate> eligible;
    std::copy_if(
        live_candidates.begin(), live_candidates.end(),
        std::back_inserter(eligible),
        [&entry](const extensions::CapabilityCandidate& candidate) {
          if (entry.ownership == ProfileOwnership::registry_managed) {
            return candidate.extension_class !=
                   extensions::ExtensionClass::protected_safety_module;
          }
          return candidate.extension_class ==
                     extensions::ExtensionClass::protected_safety_module &&
                 candidate.assigned_trust == extensions::TrustClass::built_in &&
                 candidate.controller ==
                     devices::ControllerIdentity::stm32_safety;
        });

    if (eligible.empty()) {
      if (candidates.empty()) {
        entry_result.status =
            entry.requirement == ProfileRequirement::required
                ? ProfileEntryStatus::required_missing
                : ProfileEntryStatus::optional_missing;
      } else if (profile_candidates.empty()) {
        entry_result.status = ProfileEntryStatus::wrong_profile;
        entry_result.reason_package_id = candidates.front().package_id;
      } else if (capability_candidates.empty()) {
        entry_result.status = ProfileEntryStatus::unsafe_capability;
        entry_result.reason_package_id = profile_candidates.front().package_id;
      } else if (category_candidates.empty()) {
        entry_result.status = ProfileEntryStatus::incompatible_category;
        entry_result.reason_package_id = capability_candidates.front().package_id;
      } else if (constraint_candidates.empty()) {
        entry_result.status = ProfileEntryStatus::incompatible_constraints;
        entry_result.reason_package_id = category_candidates.front().package_id;
      } else if (live_candidates.empty()) {
        entry_result.status = ProfileEntryStatus::inactive;
        entry_result.reason_package_id = constraint_candidates.front().package_id;
      } else {
        entry_result.status = ProfileEntryStatus::protected_ownership;
        entry_result.reason_package_id = live_candidates.front().package_id;
      }
    } else if (eligible.size() > max_matches_per_entry_ ||
               (single_cardinality(entry.cardinality) && eligible.size() > 1)) {
      entry_result.status = ProfileEntryStatus::ambiguous;
      entry_result.reason_package_id = eligible.front().package_id;
    } else {
      entry_result.status = ProfileEntryStatus::resolved;
      for (const auto& candidate : eligible) {
        entry_result.devices.push_back(
            {candidate.package_id, candidate.logical_device_instance_id,
             candidate.capability,
             candidate.authorization_generation});
      }
    }
    const bool optional_unavailable =
        entry.requirement == ProfileRequirement::optional &&
        (entry_result.status == ProfileEntryStatus::wrong_profile ||
         entry_result.status == ProfileEntryStatus::unsafe_capability ||
         entry_result.status == ProfileEntryStatus::incompatible_category ||
         entry_result.status == ProfileEntryStatus::incompatible_constraints ||
         entry_result.status == ProfileEntryStatus::inactive ||
         entry_result.status == ProfileEntryStatus::protected_ownership);
    if (entry_result.status != ProfileEntryStatus::resolved &&
        entry_result.status != ProfileEntryStatus::optional_missing &&
        !optional_unavailable) {
      resolution.ready = false;
    }
    resolution.entries.push_back(std::move(entry_result));
  }
  record->resolution = std::move(resolution);
  record->state = HardwareProfileState::resolved;
  return HardwareProfileResult::resolved;
}

bool HardwareProfileManager::resolution_is_current(Record& record) {
  if (!record.resolution.has_value() || !record.resolution->ready) return false;
  bool current = true;
  for (std::size_t index = 0; index < record.resolution->entries.size();
       ++index) {
    auto& entry_resolution = record.resolution->entries[index];
    if (entry_resolution.status != ProfileEntryStatus::resolved) continue;
    const auto& definition_entry = record.definition.entries[index];
    for (const auto& device : entry_resolution.devices) {
      const auto* extension = registry_.find(device.package_id);
      if (extension == nullptr ||
          extension->device_identity.logical.instance_id !=
              device.logical_device_instance_id ||
          extension->device_identity.hardware_profile.profile_id !=
              record.definition.profile_id ||
          extension->authorization_generation !=
              device.authorization_generation ||
          extension->lifecycle != extensions::LifecycleState::active ||
          !capability_matches_domain(device.active_capability,
                                     definition_entry.domain) ||
          raw_namespace(device.active_capability) ||
          !contains(extension->active_capabilities, device.active_capability) ||
          extension->manifest.category !=
              definition_entry.compatible_category) {
        entry_resolution.status = ProfileEntryStatus::revoked;
        entry_resolution.reason_package_id = device.package_id;
        current = false;
        break;
      }
    }
  }
  if (!current) {
    record.resolution->ready = false;
    record.state = HardwareProfileState::invalidated;
    if (active_profile_id_ == record.definition.profile_id) {
      active_profile_id_.clear();
    }
  }
  return current;
}

HardwareProfileResult HardwareProfileManager::activate_profile(
    const std::string& profile_id) {
  auto* record = find_mutable(profile_id);
  if (record == nullptr) return HardwareProfileResult::rejected_not_found;
  if (record->state != HardwareProfileState::resolved ||
      !record->resolution.has_value() || !record->resolution->ready) {
    return HardwareProfileResult::rejected_unresolved;
  }
  if (!resolution_is_current(*record)) {
    return HardwareProfileResult::rejected_unresolved;
  }
  if (!active_profile_id_.empty() && active_profile_id_ != profile_id) {
    auto* previous = find_mutable(active_profile_id_);
    if (previous != nullptr && previous->state == HardwareProfileState::active) {
      previous->state = HardwareProfileState::resolved;
    }
  }
  active_profile_id_ = profile_id;
  record->state = HardwareProfileState::active;
  return HardwareProfileResult::activated;
}

std::optional<HardwareProfileResolution> HardwareProfileManager::resolution(
    const std::string& profile_id) {
  auto* record = find_mutable(profile_id);
  if (record != nullptr && record->resolution.has_value() &&
      record->resolution->ready) {
    resolution_is_current(*record);
  }
  return record == nullptr ? std::nullopt : record->resolution;
}

std::vector<HardwareProfileIssue> HardwareProfileManager::validation_issues(
    const std::string& profile_id) const {
  const auto* record = find(profile_id);
  return record == nullptr ? std::vector<HardwareProfileIssue>{}
                           : record->validation_issues;
}

std::optional<HardwareProfileResolution>
HardwareProfileManager::active_profile() {
  if (active_profile_id_.empty()) return std::nullopt;
  auto* record = find_mutable(active_profile_id_);
  if (record == nullptr || !resolution_is_current(*record)) {
    return std::nullopt;
  }
  return record->resolution;
}

HardwareProfileState HardwareProfileManager::state(
    const std::string& profile_id) {
  auto* record = find_mutable(profile_id);
  if (record != nullptr && record->resolution.has_value() &&
      record->resolution->ready) {
    resolution_is_current(*record);
  }
  return record == nullptr ? HardwareProfileState::invalidated : record->state;
}

}  // namespace zie::core
