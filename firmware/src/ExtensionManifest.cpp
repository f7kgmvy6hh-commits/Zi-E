#include "zie/extensions/ExtensionManifest.hpp"

#include <algorithm>
#include <cctype>

namespace zie::extensions {
namespace {

bool compatible(const ContractVersion required, const ContractVersion supported) {
  return required.major != 0 && required.major == supported.major &&
         required.minor <= supported.minor;
}

bool nonzero_version(const ContractVersion version) {
  return version.major != 0 || version.minor != 0 || version.patch != 0;
}

bool valid_key(const std::string& value) {
  if (value.empty() || !std::isalnum(static_cast<unsigned char>(value.front()))) {
    return false;
  }
  return std::all_of(value.begin(), value.end(), [](const unsigned char character) {
    return std::islower(character) || std::isdigit(character) ||
           character == '.' || character == '_' || character == '-';
  });
}

template <typename T>
bool contains(const std::vector<T>& values, const T& value) {
  return std::find(values.begin(), values.end(), value) != values.end();
}

template <typename T>
bool has_duplicate(const std::vector<T>& values) {
  for (std::size_t index = 0; index < values.size(); ++index) {
    if (std::find(values.begin() + static_cast<std::ptrdiff_t>(index + 1),
                  values.end(), values[index]) != values.end()) {
      return true;
    }
  }
  return false;
}

void add_issue(ValidationResult& result, const ValidationIssue issue) {
  if (!contains(result.issues, issue)) result.issues.push_back(issue);
}

bool is_known(const ExtensionClass value) {
  switch (value) {
    case ExtensionClass::host_plugin:
    case ExtensionClass::asset_pack:
    case ExtensionClass::embedded_module:
    case ExtensionClass::protected_safety_module:
      return true;
  }
  return false;
}

bool is_known(const TrustClass value) {
  switch (value) {
    case TrustClass::built_in:
    case TrustClass::signed_trusted:
    case TrustClass::local_developer:
    case TrustClass::community_untrusted:
      return true;
  }
  return false;
}

bool is_known(const ControllerTarget value) {
  switch (value) {
    case ControllerTarget::none:
    case ControllerTarget::host:
    case ControllerTarget::esp32_s3:
    case ControllerTarget::stm32:
      return true;
  }
  return false;
}

bool is_known(const ExtensionCategory value) {
  switch (value) {
    case ExtensionCategory::hardware_display:
    case ExtensionCategory::hardware_camera:
    case ExtensionCategory::hardware_audio_input:
    case ExtensionCategory::hardware_audio_output:
    case ExtensionCategory::hardware_range:
    case ExtensionCategory::hardware_proximity:
    case ExtensionCategory::hardware_cliff:
    case ExtensionCategory::hardware_imu:
    case ExtensionCategory::hardware_drive:
    case ExtensionCategory::hardware_motor_driver:
    case ExtensionCategory::hardware_actuator:
    case ExtensionCategory::hardware_arm:
    case ExtensionCategory::hardware_gripper:
    case ExtensionCategory::hardware_tool:
    case ExtensionCategory::hardware_power:
    case ExtensionCategory::hardware_battery:
    case ExtensionCategory::hardware_rgb:
    case ExtensionCategory::hardware_touch:
    case ExtensionCategory::hardware_network:
    case ExtensionCategory::behavior:
    case ExtensionCategory::ai_provider:
    case ExtensionCategory::ai_tool:
    case ExtensionCategory::voice_stt:
    case ExtensionCategory::voice_tts:
    case ExtensionCategory::voice_wakeword:
    case ExtensionCategory::face_pack:
    case ExtensionCategory::emotion_pack:
    case ExtensionCategory::rgb_pack:
    case ExtensionCategory::sound_pack:
    case ExtensionCategory::automation:
    case ExtensionCategory::integration:
      return true;
  }
  return false;
}

bool is_known(const Permission value) {
  switch (value) {
    case Permission::read_robot_state:
    case Permission::read_camera:
    case Permission::read_microphone:
    case Permission::write_face:
    case Permission::write_rgb:
    case Permission::play_audio:
    case Permission::request_motion:
    case Permission::access_network:
    case Permission::access_storage:
      return true;
  }
  return false;
}

bool is_known(const LifecycleState value) {
  switch (value) {
    case LifecycleState::discovered:
    case LifecycleState::validating:
    case LifecycleState::installed:
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

bool is_known(const FailureClass value) {
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

bool lifecycle_may_expose_active_capabilities(const LifecycleState lifecycle) {
  return lifecycle == LifecycleState::active ||
         lifecycle == LifecycleState::degraded;
}

bool lifecycle_may_report_failure(const LifecycleState lifecycle) {
  return lifecycle == LifecycleState::degraded ||
         lifecycle == LifecycleState::failed ||
         lifecycle == LifecycleState::quarantined;
}

bool is_asset_category(const ExtensionCategory category) {
  switch (category) {
    case ExtensionCategory::face_pack:
    case ExtensionCategory::emotion_pack:
    case ExtensionCategory::rgb_pack:
    case ExtensionCategory::sound_pack:
      return true;
    case ExtensionCategory::hardware_display:
    case ExtensionCategory::hardware_camera:
    case ExtensionCategory::hardware_audio_input:
    case ExtensionCategory::hardware_audio_output:
    case ExtensionCategory::hardware_range:
    case ExtensionCategory::hardware_proximity:
    case ExtensionCategory::hardware_cliff:
    case ExtensionCategory::hardware_imu:
    case ExtensionCategory::hardware_drive:
    case ExtensionCategory::hardware_motor_driver:
    case ExtensionCategory::hardware_actuator:
    case ExtensionCategory::hardware_arm:
    case ExtensionCategory::hardware_gripper:
    case ExtensionCategory::hardware_tool:
    case ExtensionCategory::hardware_power:
    case ExtensionCategory::hardware_battery:
    case ExtensionCategory::hardware_rgb:
    case ExtensionCategory::hardware_touch:
    case ExtensionCategory::hardware_network:
    case ExtensionCategory::behavior:
    case ExtensionCategory::ai_provider:
    case ExtensionCategory::ai_tool:
    case ExtensionCategory::voice_stt:
    case ExtensionCategory::voice_tts:
    case ExtensionCategory::voice_wakeword:
    case ExtensionCategory::automation:
    case ExtensionCategory::integration:
      return false;
  }
  return false;
}

bool is_hardware_category(const ExtensionCategory category) {
  switch (category) {
    case ExtensionCategory::hardware_display:
    case ExtensionCategory::hardware_camera:
    case ExtensionCategory::hardware_audio_input:
    case ExtensionCategory::hardware_audio_output:
    case ExtensionCategory::hardware_range:
    case ExtensionCategory::hardware_proximity:
    case ExtensionCategory::hardware_cliff:
    case ExtensionCategory::hardware_imu:
    case ExtensionCategory::hardware_drive:
    case ExtensionCategory::hardware_motor_driver:
    case ExtensionCategory::hardware_actuator:
    case ExtensionCategory::hardware_arm:
    case ExtensionCategory::hardware_gripper:
    case ExtensionCategory::hardware_tool:
    case ExtensionCategory::hardware_power:
    case ExtensionCategory::hardware_battery:
    case ExtensionCategory::hardware_rgb:
    case ExtensionCategory::hardware_touch:
    case ExtensionCategory::hardware_network:
      return true;
    case ExtensionCategory::behavior:
    case ExtensionCategory::ai_provider:
    case ExtensionCategory::ai_tool:
    case ExtensionCategory::voice_stt:
    case ExtensionCategory::voice_tts:
    case ExtensionCategory::voice_wakeword:
    case ExtensionCategory::face_pack:
    case ExtensionCategory::emotion_pack:
    case ExtensionCategory::rgb_pack:
    case ExtensionCategory::sound_pack:
    case ExtensionCategory::automation:
    case ExtensionCategory::integration:
      return false;
  }
  return false;
}

}  // namespace

ValidationResult validate_manifest(const ExtensionManifest& manifest,
                                   const ManifestValidationContext& context) {
  ValidationResult result;
  const bool known_extension_class = is_known(manifest.extension_class);
  if (!known_extension_class) {
    add_issue(result, ValidationIssue::unknown_extension_class);
  }
  if (!is_known(manifest.category)) {
    add_issue(result, ValidationIssue::unknown_extension_category);
  }
  if (!is_known(context.assigned_trust)) {
    add_issue(result, ValidationIssue::unknown_trust_class);
  }
  if (!is_known(manifest.controller_target)) {
    add_issue(result, ValidationIssue::unknown_controller_target);
  }
  if (!valid_key(manifest.id)) add_issue(result, ValidationIssue::invalid_id);
  if (manifest.name.empty()) add_issue(result, ValidationIssue::invalid_name);
  if (!nonzero_version(manifest.version)) {
    add_issue(result, ValidationIssue::invalid_version);
  }
  if (!compatible(manifest.plugin_api_version,
                  supported_plugin_api_version())) {
    add_issue(result, ValidationIssue::incompatible_plugin_api);
  }
  if (!compatible(manifest.manifest_schema_version,
                  supported_manifest_schema_version())) {
    add_issue(result, ValidationIssue::incompatible_manifest_schema);
  }
  if (has_duplicate(manifest.declared_capabilities)) {
    add_issue(result, ValidationIssue::duplicate_declared_capability);
  }
  for (const auto& capability : manifest.declared_capabilities) {
    if (!valid_key(capability)) {
      add_issue(result, ValidationIssue::invalid_declared_capability);
    }
  }
  for (const auto permission : manifest.required_permissions) {
    if (!is_known(permission)) {
      add_issue(result, ValidationIssue::unknown_permission);
    }
  }
  if (has_duplicate(manifest.required_permissions)) {
    add_issue(result, ValidationIssue::duplicate_permission);
  }

  if (known_extension_class) switch (manifest.extension_class) {
    case ExtensionClass::host_plugin:
      if (is_asset_category(manifest.category)) {
        add_issue(result, ValidationIssue::invalid_category_for_class);
      }
      if (manifest.controller_target != ControllerTarget::host) {
        add_issue(result, ValidationIssue::invalid_controller_target);
      }
      if (manifest.entrypoint.empty()) {
        add_issue(result, ValidationIssue::missing_entrypoint);
      }
      break;
    case ExtensionClass::asset_pack:
      if (!is_asset_category(manifest.category)) {
        add_issue(result, ValidationIssue::invalid_category_for_class);
      }
      if (manifest.controller_target != ControllerTarget::none) {
        add_issue(result, ValidationIssue::invalid_controller_target);
      }
      if (!manifest.entrypoint.empty()) {
        add_issue(result, ValidationIssue::forbidden_entrypoint);
      }
      if (!manifest.required_permissions.empty()) {
        add_issue(result, ValidationIssue::forbidden_permission);
      }
      break;
    case ExtensionClass::embedded_module:
      if (!is_hardware_category(manifest.category)) {
        add_issue(result, ValidationIssue::invalid_category_for_class);
      }
      if (manifest.controller_target != ControllerTarget::esp32_s3 &&
          manifest.controller_target != ControllerTarget::stm32) {
        add_issue(result, ValidationIssue::invalid_controller_target);
      }
      if (manifest.entrypoint.empty()) {
        add_issue(result, ValidationIssue::missing_entrypoint);
      }
      if (!manifest.required_permissions.empty()) {
        add_issue(result, ValidationIssue::forbidden_permission);
      }
      break;
    case ExtensionClass::protected_safety_module:
      if (!is_hardware_category(manifest.category)) {
        add_issue(result, ValidationIssue::invalid_category_for_class);
      }
      if (manifest.controller_target != ControllerTarget::stm32) {
        add_issue(result, ValidationIssue::invalid_controller_target);
      }
      if (context.assigned_trust != TrustClass::built_in) {
        add_issue(result,
                  ValidationIssue::protected_module_requires_built_in_trust);
      }
      if (manifest.entrypoint.empty()) {
        add_issue(result, ValidationIssue::missing_entrypoint);
      }
      if (!manifest.required_permissions.empty()) {
        add_issue(result, ValidationIssue::forbidden_permission);
      }
      break;
  }

  result.valid = result.issues.empty();
  return result;
}

ValidationResult validate_capability_state(const ExtensionManifest& manifest,
                                           const CapabilityState& state) {
  ValidationResult result;
  const bool known_lifecycle = is_known(state.lifecycle);
  const bool known_failure = is_known(state.failure);
  if (!known_lifecycle) {
    add_issue(result, ValidationIssue::unknown_lifecycle_state);
  }
  if (!known_failure) {
    add_issue(result, ValidationIssue::unknown_failure_class);
  }
  if (has_duplicate(state.validated_capabilities)) {
    add_issue(result, ValidationIssue::duplicate_validated_capability);
  }
  for (const auto& capability : state.validated_capabilities) {
    if (!contains(manifest.declared_capabilities, capability)) {
      add_issue(result, ValidationIssue::undeclared_validated_capability);
    }
  }
  if (has_duplicate(state.active_capabilities)) {
    add_issue(result, ValidationIssue::duplicate_active_capability);
  }
  for (const auto& capability : state.active_capabilities) {
    if (!contains(state.validated_capabilities, capability)) {
      add_issue(result, ValidationIssue::unvalidated_active_capability);
    }
  }
  if (known_lifecycle && !state.active_capabilities.empty() &&
      !lifecycle_may_expose_active_capabilities(state.lifecycle)) {
    add_issue(result,
              ValidationIssue::active_capability_in_inactive_lifecycle);
  }
  if (known_lifecycle && known_failure &&
      state.lifecycle == LifecycleState::failed &&
      state.failure == FailureClass::none) {
    add_issue(result, ValidationIssue::failure_state_without_failure_class);
  }
  if (known_lifecycle && known_failure &&
      !lifecycle_may_report_failure(state.lifecycle) &&
      state.failure != FailureClass::none) {
    add_issue(result, ValidationIssue::failure_class_without_failure_state);
  }
  result.valid = result.issues.empty();
  return result;
}

}  // namespace zie::extensions
