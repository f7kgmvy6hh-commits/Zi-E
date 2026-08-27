#include <cassert>
#include <string>
#include <vector>

#include "zie/api/SemanticRobotApi.hpp"

namespace {
using namespace zie::api;
using namespace zie::extensions;
using zie::devices::ControllerIdentity;

ExtensionCandidate candidate() {
  ExtensionCandidate value;
  value.manifest.id = "zie.semantic.plugin";
  value.manifest.name = "Semantic Plugin";
  value.manifest.version = {1, 0, 0};
  value.manifest.plugin_api_version = supported_plugin_api_version();
  value.manifest.manifest_schema_version =
      supported_manifest_schema_version();
  value.manifest.category = ExtensionCategory::integration;
  value.manifest.extension_class = ExtensionClass::host_plugin;
  value.manifest.controller_target = ControllerTarget::host;
  value.manifest.entrypoint = "create_semantic_plugin";
  value.manifest.declared_capabilities = {
      "semantic.motion", "semantic.presentation", "semantic.audio",
      "semantic.sensor-query"};
  value.manifest.required_permissions = {Permission::request_motion,
                                         Permission::write_face,
                                         Permission::play_audio,
                                         Permission::read_robot_state};
  value.device_identity.package.extension_id = value.manifest.id;
  value.device_identity.physical.state =
      zie::devices::PhysicalIdentityState::manufacturer_serial;
  value.device_identity.physical.manufacturer = "ZI-E Test";
  value.device_identity.physical.model = "Semantic Fixture";
  value.device_identity.physical.serial = "semantic-001";
  value.device_identity.physical.provenance = {
      zie::devices::IdentitySource::operator_verified,
      zie::devices::IdentityTrust::verified};
  value.device_identity.logical.instance_id = "semantic.plugin.primary";
  value.device_identity.controller.controller = ControllerIdentity::host;
  value.device_identity.hardware_profile.profile_id = "zie.test-profile";
  return value;
}

void register_active(ExtensionRegistry& registry,
                     const ExtensionCandidate& owner,
                     const std::vector<std::string>& active_capabilities) {
  const RegistryAssignment assignment{owner.manifest.id,
                                      TrustClass::local_developer,
                                      ControllerIdentity::host,
                                      "zie.test-profile"};
  assert(registry.register_extension(owner, assignment) ==
         RegistryResult::accepted);
  assert(registry.transition(owner.manifest.id, LifecycleState::validated) ==
         RegistryResult::transitioned);
  assert(registry.validate_capabilities(owner.manifest.id,
                                        active_capabilities) ==
         RegistryResult::capabilities_validated);
  assert(registry.transition(owner.manifest.id, LifecycleState::inactive) ==
         RegistryResult::transitioned);
  assert(registry.transition(owner.manifest.id, LifecycleState::activating) ==
         RegistryResult::transitioned);
  assert(registry.activate_capabilities(owner.manifest.id,
                                        active_capabilities) ==
         RegistryResult::capabilities_activated);
}

CommandSourceIdentity source(const ExtensionCandidate& owner,
                             const std::uint64_t sequence = 1) {
  return {owner.manifest.id, owner.device_identity.logical.instance_id, 10,
          sequence};
}
}  // namespace

void run_semantic_robot_api_tests() {
  const auto owner = candidate();
  ExtensionRegistry active_registry;
  register_active(active_registry, owner,
                  {"semantic.motion", "semantic.presentation",
                   "semantic.audio", "semantic.sensor-query"});
  SemanticRobotApi api(active_registry);
  SemanticCommand motion{SemanticCommandType::motion_intent, source(owner),
                         MotionIntent{0.2F, -0.1F, 100}};
  assert(api.submit(motion) == CommandResult::accepted);
  assert(api.submit(motion) == CommandResult::rejected_stale_sequence);

  ExtensionRegistry inactive_registry;
  register_active(inactive_registry, owner, {"semantic.motion"});
  assert(inactive_registry.transition(owner.manifest.id,
                                      LifecycleState::inactive) ==
         RegistryResult::transitioned);
  SemanticRobotApi inactive_api(inactive_registry);
  assert(inactive_api.submit(motion) ==
         CommandResult::rejected_inactive_issuer);

  ExtensionRegistry missing_capability_registry;
  register_active(missing_capability_registry, owner,
                  {"semantic.presentation"});
  SemanticRobotApi missing_capability_api(missing_capability_registry);
  assert(missing_capability_api.submit(motion) ==
         CommandResult::rejected_missing_capability);

  auto impersonated = motion;
  impersonated.source.package_id = "zie.semantic.victim";
  assert(api.submit(impersonated) == CommandResult::rejected_invalid_identity);
  impersonated = motion;
  impersonated.source.logical_device_instance_id = "victim.device";
  assert(api.submit(impersonated) == CommandResult::rejected_invalid_identity);

  auto raw_style = motion;
  raw_style.type = static_cast<SemanticCommandType>(999);
  raw_style.payload = ExpressionIntent{"pwm=255 gpio=1 raw-can-frame"};
  assert(api.submit(raw_style) == CommandResult::rejected_unknown_command);

  SemanticCommand safety_bypass{
      SemanticCommandType::protected_safety_operation, source(owner, 2),
      ProtectedSafetyOperation{"disable-cliff-interlock"}};
  assert(api.submit(safety_bypass) ==
         CommandResult::rejected_protected_operation);

  EventJournal events;
  RobotStateStore state;
  const AuthoritativeRobotCore core;
  assert(core.update_state(state,
                           {RobotStateCategory::motion, 1, "stopped"}) ==
         StateResult::accepted);
  const auto before_event = *state.current(RobotStateCategory::motion);
  assert(events.publish({1, EventCategory::sensor_observation,
                         EventSourceType::extension, owner.manifest.id,
                         "range observation available"}) ==
         EventResult::published);
  assert(state.current(RobotStateCategory::motion)->generation ==
         before_event.generation);
  assert(state.current(RobotStateCategory::motion)->semantic_value ==
         before_event.semantic_value);

  assert(core.update_state(state,
                           {RobotStateCategory::motion, 1, "moving"}) ==
         StateResult::rejected_stale_generation);
  assert(core.update_state(state,
                           {RobotStateCategory::motion, 0, "moving"}) ==
         StateResult::rejected_invalid_state);
  assert(state.current(RobotStateCategory::motion)->semantic_value ==
         "stopped");

  assert(events.publish({2, static_cast<EventCategory>(999),
                         EventSourceType::authoritative_core, "core",
                         "unknown"}) ==
         EventResult::rejected_unknown_category);
  assert(events.publish({2, EventCategory::command_rejected,
                         static_cast<EventSourceType>(999), "core",
                         "unknown"}) == EventResult::rejected_unknown_source);
  assert(core.update_state(
             state, {static_cast<RobotStateCategory>(999), 2, "unknown"}) ==
         StateResult::rejected_unknown_category);
}
