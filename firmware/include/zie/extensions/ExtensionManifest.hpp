#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace zie::extensions {

struct ContractVersion {
  std::uint16_t major{0};
  std::uint16_t minor{0};
  std::uint16_t patch{0};
};

enum class ExtensionClass {
  host_plugin,
  asset_pack,
  embedded_module,
  protected_safety_module,
};

enum class TrustClass {
  built_in,
  signed_trusted,
  local_developer,
  community_untrusted,
};

enum class ControllerTarget { none, host, esp32_s3, stm32 };

enum class ExtensionCategory {
  hardware_display,
  hardware_camera,
  hardware_audio_input,
  hardware_audio_output,
  hardware_range,
  hardware_proximity,
  hardware_cliff,
  hardware_imu,
  hardware_drive,
  hardware_motor_driver,
  hardware_actuator,
  hardware_arm,
  hardware_gripper,
  hardware_tool,
  hardware_power,
  hardware_battery,
  hardware_rgb,
  hardware_touch,
  hardware_network,
  behavior,
  ai_provider,
  ai_tool,
  voice_stt,
  voice_tts,
  voice_wakeword,
  face_pack,
  emotion_pack,
  rgb_pack,
  sound_pack,
  automation,
  integration,
};

enum class Permission {
  read_robot_state,
  read_camera,
  read_microphone,
  write_face,
  write_rgb,
  play_audio,
  request_motion,
  access_network,
  access_storage,
};

enum class LifecycleState {
  discovered,
  validating,
  installed,
  validated,
  inactive,
  activating,
  configured,
  commissioning,
  active,
  degraded,
  failed,
  quarantined,
  disabled,
  removed,
};

enum class FailureClass {
  none,
  temporary,
  configuration,
  authentication,
  incompatible,
  hardware_fault,
  security,
};

struct ExtensionManifest {
  std::string id;
  std::string name;
  ContractVersion version{};
  ContractVersion plugin_api_version{};
  ContractVersion manifest_schema_version{};
  ExtensionCategory category{ExtensionCategory::behavior};
  ExtensionClass extension_class{ExtensionClass::host_plugin};
  ControllerTarget controller_target{ControllerTarget::none};
  std::string entrypoint;
  std::vector<std::string> declared_capabilities;
  std::vector<Permission> required_permissions;
};

// Trust is assigned by the registry/installation source, never claimed by a manifest.
struct ManifestValidationContext {
  TrustClass assigned_trust{TrustClass::community_untrusted};
};

struct CapabilityState {
  LifecycleState lifecycle{LifecycleState::discovered};
  FailureClass failure{FailureClass::none};
  std::vector<std::string> validated_capabilities;
  std::vector<std::string> active_capabilities;
};

enum class ValidationIssue {
  invalid_id,
  invalid_name,
  invalid_version,
  incompatible_plugin_api,
  incompatible_manifest_schema,
  unknown_extension_class,
  unknown_extension_category,
  unknown_permission,
  unknown_trust_class,
  unknown_controller_target,
  unknown_lifecycle_state,
  unknown_failure_class,
  invalid_category_for_class,
  invalid_controller_target,
  missing_entrypoint,
  forbidden_entrypoint,
  forbidden_permission,
  duplicate_permission,
  protected_module_requires_built_in_trust,
  duplicate_declared_capability,
  invalid_declared_capability,
  duplicate_validated_capability,
  undeclared_validated_capability,
  duplicate_active_capability,
  unvalidated_active_capability,
  active_capability_in_inactive_lifecycle,
  failure_class_without_failure_state,
  failure_state_without_failure_class,
};

struct ValidationResult {
  bool valid{false};
  std::vector<ValidationIssue> issues;
};

constexpr ContractVersion supported_plugin_api_version() { return {1, 0, 0}; }
constexpr ContractVersion supported_manifest_schema_version() {
  return {1, 0, 0};
}

ValidationResult validate_manifest(const ExtensionManifest& manifest,
                                   const ManifestValidationContext& context);
ValidationResult validate_capability_state(const ExtensionManifest& manifest,
                                           const CapabilityState& state);

}  // namespace zie::extensions
