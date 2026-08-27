#include <algorithm>
#include <cassert>

#include "zie/extensions/ExtensionManifest.hpp"

namespace {

using namespace zie::extensions;

bool has_issue(const ValidationResult& result, const ValidationIssue issue) {
  return std::find(result.issues.begin(), result.issues.end(), issue) !=
         result.issues.end();
}

ExtensionManifest host_manifest() {
  ExtensionManifest manifest;
  manifest.id = "zie.voice.local-tts";
  manifest.name = "Local TTS";
  manifest.version = {1, 2, 0};
  manifest.plugin_api_version = supported_plugin_api_version();
  manifest.manifest_schema_version = supported_manifest_schema_version();
  manifest.category = ExtensionCategory::voice_tts;
  manifest.extension_class = ExtensionClass::host_plugin;
  manifest.controller_target = ControllerTarget::host;
  manifest.entrypoint = "local_tts:create_provider";
  manifest.declared_capabilities = {"voice.tts", "voice.streaming"};
  manifest.required_permissions = {Permission::play_audio};
  return manifest;
}

}  // namespace

void run_extension_manifest_tests() {
  const ManifestValidationContext local_developer{
      TrustClass::local_developer};
  auto host = host_manifest();
  assert(validate_manifest(host, local_developer).valid);

  CapabilityState discovered;
  assert(validate_capability_state(host, discovered).valid);
  assert(discovered.validated_capabilities.empty());
  assert(discovered.active_capabilities.empty());

  CapabilityState validated;
  validated.lifecycle = LifecycleState::configured;
  validated.validated_capabilities = {"voice.tts"};
  assert(validate_capability_state(host, validated).valid);
  assert(validated.active_capabilities.empty());

  CapabilityState active = validated;
  active.lifecycle = LifecycleState::active;
  active.active_capabilities = {"voice.tts"};
  assert(validate_capability_state(host, active).valid);

  auto incompatible = host;
  incompatible.plugin_api_version = {2, 0, 0};
  auto incompatible_result = validate_manifest(incompatible, local_developer);
  assert(!incompatible_result.valid);
  assert(has_issue(incompatible_result,
                   ValidationIssue::incompatible_plugin_api));

  auto future_schema = host;
  future_schema.manifest_schema_version = {1, 1, 0};
  auto schema_result = validate_manifest(future_schema, local_developer);
  assert(!schema_result.valid);
  assert(has_issue(schema_result,
                   ValidationIssue::incompatible_manifest_schema));

  auto duplicate_declaration = host;
  duplicate_declaration.declared_capabilities.push_back("voice.tts");
  auto duplicate_result =
      validate_manifest(duplicate_declaration, local_developer);
  assert(has_issue(duplicate_result,
                   ValidationIssue::duplicate_declared_capability));

  auto asset = host;
  asset.id = "zie.face.soft";
  asset.category = ExtensionCategory::face_pack;
  asset.extension_class = ExtensionClass::asset_pack;
  asset.controller_target = ControllerTarget::none;
  asset.entrypoint.clear();
  asset.required_permissions.clear();
  assert(validate_manifest(asset, local_developer).valid);
  asset.entrypoint = "assets:execute";
  asset.required_permissions = {Permission::request_motion};
  auto executable_asset = validate_manifest(asset, local_developer);
  assert(has_issue(executable_asset, ValidationIssue::forbidden_entrypoint));
  assert(has_issue(executable_asset, ValidationIssue::forbidden_permission));
  asset.entrypoint.clear();
  asset.required_permissions.clear();
  asset.category = ExtensionCategory::behavior;
  assert(has_issue(validate_manifest(asset, local_developer),
                   ValidationIssue::invalid_category_for_class));

  auto embedded = host;
  embedded.id = "zie.hardware.display.mock";
  embedded.category = ExtensionCategory::hardware_display;
  embedded.extension_class = ExtensionClass::embedded_module;
  embedded.controller_target = ControllerTarget::esp32_s3;
  embedded.required_permissions.clear();
  assert(validate_manifest(embedded, local_developer).valid);
  embedded.controller_target = ControllerTarget::host;
  assert(has_issue(validate_manifest(embedded, local_developer),
                   ValidationIssue::invalid_controller_target));
  embedded.controller_target = ControllerTarget::esp32_s3;
  embedded.category = ExtensionCategory::ai_provider;
  assert(has_issue(validate_manifest(embedded, local_developer),
                   ValidationIssue::invalid_category_for_class));

  auto protected_module = host;
  protected_module.id = "zie.safety.motion-supervisor";
  protected_module.category = ExtensionCategory::hardware_drive;
  protected_module.extension_class = ExtensionClass::protected_safety_module;
  protected_module.controller_target = ControllerTarget::stm32;
  protected_module.required_permissions.clear();
  const ManifestValidationContext built_in{TrustClass::built_in};
  assert(validate_manifest(protected_module, built_in).valid);
  assert(has_issue(
      validate_manifest(protected_module, local_developer),
      ValidationIssue::protected_module_requires_built_in_trust));

  CapabilityState undeclared;
  undeclared.lifecycle = LifecycleState::configured;
  undeclared.validated_capabilities = {"voice.unknown"};
  assert(has_issue(validate_capability_state(host, undeclared),
                   ValidationIssue::undeclared_validated_capability));

  CapabilityState unvalidated;
  unvalidated.lifecycle = LifecycleState::active;
  unvalidated.active_capabilities = {"voice.tts"};
  assert(has_issue(validate_capability_state(host, unvalidated),
                   ValidationIssue::unvalidated_active_capability));

  CapabilityState disabled = active;
  disabled.lifecycle = LifecycleState::disabled;
  assert(has_issue(
      validate_capability_state(host, disabled),
      ValidationIssue::active_capability_in_inactive_lifecycle));

  CapabilityState failed;
  failed.lifecycle = LifecycleState::failed;
  assert(has_issue(validate_capability_state(host, failed),
                   ValidationIssue::failure_state_without_failure_class));
  failed.failure = FailureClass::hardware_fault;
  assert(validate_capability_state(host, failed).valid);

  auto motion_requester = host;
  motion_requester.required_permissions = {Permission::request_motion};
  assert(validate_manifest(motion_requester, local_developer).valid);
  // request_motion denotes Safe Robot API access only; no raw actuator permission
  // exists in the manifest vocabulary.
  motion_requester.required_permissions.push_back(Permission::request_motion);
  assert(has_issue(validate_manifest(motion_requester, local_developer),
                   ValidationIssue::duplicate_permission));

  auto unknown_class = host;
  unknown_class.extension_class = static_cast<ExtensionClass>(999);
  unknown_class.required_permissions = {static_cast<Permission>(999)};
  auto unknown_class_result =
      validate_manifest(unknown_class, local_developer);
  assert(!unknown_class_result.valid);
  assert(has_issue(unknown_class_result,
                   ValidationIssue::unknown_extension_class));
  assert(has_issue(unknown_class_result, ValidationIssue::unknown_permission));

  auto unknown_category = host;
  unknown_category.category = static_cast<ExtensionCategory>(999);
  auto unknown_category_result =
      validate_manifest(unknown_category, local_developer);
  assert(!unknown_category_result.valid);
  assert(has_issue(unknown_category_result,
                   ValidationIssue::unknown_extension_category));

  const ManifestValidationContext unknown_trust{
      static_cast<TrustClass>(999)};
  auto unknown_trust_result = validate_manifest(host, unknown_trust);
  assert(!unknown_trust_result.valid);
  assert(has_issue(unknown_trust_result,
                   ValidationIssue::unknown_trust_class));

  auto unknown_target = host;
  unknown_target.controller_target = static_cast<ControllerTarget>(999);
  auto unknown_target_result =
      validate_manifest(unknown_target, local_developer);
  assert(!unknown_target_result.valid);
  assert(has_issue(unknown_target_result,
                   ValidationIssue::unknown_controller_target));

  CapabilityState unknown_lifecycle;
  unknown_lifecycle.lifecycle = static_cast<LifecycleState>(999);
  auto unknown_lifecycle_result =
      validate_capability_state(host, unknown_lifecycle);
  assert(!unknown_lifecycle_result.valid);
  assert(has_issue(unknown_lifecycle_result,
                   ValidationIssue::unknown_lifecycle_state));

  CapabilityState unknown_failure;
  unknown_failure.failure = static_cast<FailureClass>(999);
  auto unknown_failure_result =
      validate_capability_state(host, unknown_failure);
  assert(!unknown_failure_result.valid);
  assert(has_issue(unknown_failure_result,
                   ValidationIssue::unknown_failure_class));
}
