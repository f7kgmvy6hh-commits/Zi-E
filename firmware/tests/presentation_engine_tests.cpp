#include <cassert>
#include <string>
#include <vector>

#include "core/AuthoritativeRobotCore.hpp"
#include "zie/api/VirtualRobot.hpp"
#include "zie/presentation/PresentationEngine.hpp"

namespace {
using namespace zie::api;
using namespace zie::extensions;
using namespace zie::presentation;
using zie::devices::ControllerIdentity;

ExtensionCandidate asset_candidate(const std::string& id,
                                   const std::string& logical,
                                   const std::string& serial,
                                   const ExtensionCategory category,
                                   const std::string& capability) {
  ExtensionCandidate value;
  value.manifest.id = id; value.manifest.name = id;
  value.manifest.version = {1, 0, 0};
  value.manifest.plugin_api_version = supported_plugin_api_version();
  value.manifest.manifest_schema_version = supported_manifest_schema_version();
  value.manifest.category = category;
  value.manifest.extension_class = ExtensionClass::asset_pack;
  value.manifest.controller_target = ControllerTarget::none;
  value.manifest.declared_capabilities = {capability};
  value.device_identity.package.extension_id = id;
  value.device_identity.physical.state = zie::devices::PhysicalIdentityState::manufacturer_serial;
  value.device_identity.physical.manufacturer = "ZI-E Test";
  value.device_identity.physical.model = "Data Pack";
  value.device_identity.physical.serial = serial;
  value.device_identity.physical.provenance = {zie::devices::IdentitySource::operator_verified,
                                                zie::devices::IdentityTrust::verified};
  value.device_identity.logical.instance_id = logical;
  value.device_identity.controller.controller = ControllerIdentity::none;
  value.device_identity.hardware_profile.profile_id = "zie.test-profile";
  return value;
}

void register_and_activate(ExtensionRegistry& registry,
                           const ExtensionCandidate& value) {
  assert(registry.register_extension(value, {value.manifest.id,
             TrustClass::local_developer, ControllerIdentity::none,
             "zie.test-profile"}) == RegistryResult::accepted);
  assert(registry.transition(value.manifest.id, LifecycleState::validated) == RegistryResult::transitioned);
  assert(registry.validate_capabilities(value.manifest.id,
             value.manifest.declared_capabilities) == RegistryResult::capabilities_validated);
  assert(registry.transition(value.manifest.id, LifecycleState::inactive) == RegistryResult::transitioned);
  assert(registry.transition(value.manifest.id, LifecycleState::activating) == RegistryResult::transitioned);
  assert(registry.activate_capabilities(value.manifest.id,
             value.manifest.declared_capabilities) == RegistryResult::capabilities_activated);
}

PackIdentity pack_identity(const ExtensionCandidate& value,
                           const PackDomain domain) {
  return {value.manifest.id, value.manifest.version, {1, 0, 0}, domain,
          value.manifest.id, value.device_identity.logical.instance_id,
          "zie.test-profile", value.manifest.declared_capabilities.front()};
}

FacePack face_pack(const ExtensionCandidate& value) {
  return {pack_identity(value, PackDomain::face), "neutral", "neutral",
          {{"neutral", "asset.face.neutral", {}},
           {"happy", "asset.face.happy", {{"intensity", "normal"}}}},
          {{"neutral", "happy", "crossfade"}}};
}

SoundPack sound_pack(const ExtensionCandidate& value) {
  return {pack_identity(value, PackDomain::sound),
          {{"ready", SoundCueKind::non_speech, "asset.sound.ready"},
           {"greeting", SoundCueKind::speech, "asset.sound.greeting"}}};
}

ExtensionCandidate client_candidate() {
  auto value = asset_candidate("zie.presentation.client", "presentation.client",
                               "client-001", ExtensionCategory::integration,
                               "semantic.presentation");
  value.manifest.extension_class = ExtensionClass::host_plugin;
  value.manifest.controller_target = ControllerTarget::host;
  value.manifest.entrypoint = "create_client";
  value.manifest.declared_capabilities = {"semantic.presentation", "semantic.audio"};
  value.device_identity.controller.controller = ControllerIdentity::host;
  return value;
}

void activate_client(ExtensionRegistry& registry, const ExtensionCandidate& value) {
  assert(registry.register_extension(value, {value.manifest.id,
             TrustClass::local_developer, ControllerIdentity::host,
             "zie.test-profile"}) == RegistryResult::accepted);
  assert(registry.transition(value.manifest.id, LifecycleState::validated) == RegistryResult::transitioned);
  assert(registry.validate_capabilities(value.manifest.id, value.manifest.declared_capabilities) == RegistryResult::capabilities_validated);
  assert(registry.transition(value.manifest.id, LifecycleState::inactive) == RegistryResult::transitioned);
  assert(registry.transition(value.manifest.id, LifecycleState::activating) == RegistryResult::transitioned);
  assert(registry.activate_capabilities(value.manifest.id, value.manifest.declared_capabilities) == RegistryResult::capabilities_activated);
}
}  // namespace

void run_presentation_engine_tests() {
  ExtensionRegistry registry;
  const auto face_one = asset_candidate("zie.face.soft", "face.soft.primary", "face-001",
                                        ExtensionCategory::face_pack,
                                        "presentation.face-pack.soft");
  const auto face_two = asset_candidate("zie.face.bold", "face.bold.primary", "face-002",
                                        ExtensionCategory::face_pack,
                                        "presentation.face-pack.bold");
  const auto sound = asset_candidate("zie.sound.basic", "sound.basic.primary", "sound-001",
                                     ExtensionCategory::sound_pack,
                                     "presentation.sound-pack.basic");
  register_and_activate(registry, face_one);
  register_and_activate(registry, face_two);
  register_and_activate(registry, sound);

  PackCatalog catalog(registry, 12, 8);
  auto soft = face_pack(face_one);
  auto bold = face_pack(face_two);
  bold.expressions[1].asset_handle = "asset.face.bold-happy";
  const auto basic_sound = sound_pack(sound);
  assert(catalog.declare_pack(soft) == PackResult::declared);
  assert(catalog.validate(soft.identity.pack_id) == PackResult::validated);
  PackCatalog query_catalog(registry, 1, 1);
  assert(query_catalog.declare_pack(soft) == PackResult::declared);
  assert(query_catalog.validate(soft.identity.pack_id) == PackResult::validated);
  assert(query_catalog.active_face("missing") == nullptr);
  assert(query_catalog.active_sound("missing") == nullptr);
  assert(query_catalog.activate_face("real", soft.identity.pack_id) ==
         PackResult::activated);
  assert(catalog.activate_face("face-a", soft.identity.pack_id) ==
         PackResult::activated);
  assert(catalog.activate_face("face-b", soft.identity.pack_id) ==
         PackResult::activated);
  assert(catalog.activate_face("main", soft.identity.pack_id) == PackResult::activated);
  assert(catalog.active_face("main")->identity.pack_id == soft.identity.pack_id);
  assert(catalog.declare_pack(bold) == PackResult::declared);
  assert(catalog.validate(bold.identity.pack_id) == PackResult::validated);
  assert(catalog.activate_face("face-a", bold.identity.pack_id) ==
         PackResult::replaced);
  assert(catalog.active_face("face-a")->identity.pack_id ==
         bold.identity.pack_id);
  assert(catalog.active_face("face-b")->identity.pack_id ==
         soft.identity.pack_id);
  assert(catalog.activate_face("face-c", soft.identity.pack_id) ==
         PackResult::activated);
  assert(catalog.activate_face("main", bold.identity.pack_id) == PackResult::replaced);
  assert(catalog.active_face("main")->identity.pack_id == bold.identity.pack_id);
  assert(catalog.declare_pack(basic_sound) == PackResult::declared);
  assert(catalog.validate(basic_sound.identity.pack_id) == PackResult::validated);
  assert(catalog.activate_sound("main", basic_sound.identity.pack_id) == PackResult::activated);
  assert(catalog.activate_sound("sound-a", basic_sound.identity.pack_id) ==
         PackResult::activated);
  assert(catalog.activate_sound("sound-b", basic_sound.identity.pack_id) ==
         PackResult::activated);
  assert(catalog.active_sound("sound-a")->identity.pack_id ==
         basic_sound.identity.pack_id);
  assert(catalog.active_sound("sound-b")->identity.pack_id ==
         basic_sound.identity.pack_id);

  const auto duplicate_candidate = asset_candidate("zie.face.duplicate", "face.duplicate", "face-003",
      ExtensionCategory::face_pack, "presentation.face-pack.duplicate");
  register_and_activate(registry, duplicate_candidate);
  auto duplicate_pack = face_pack(duplicate_candidate);
  duplicate_pack.expressions.push_back(duplicate_pack.expressions.front());
  assert(catalog.declare_pack(duplicate_pack) == PackResult::declared);
  assert(catalog.validate(duplicate_pack.identity.pack_id) == PackResult::rejected_duplicate_name);

  const auto missing_candidate = asset_candidate("zie.face.missing", "face.missing", "face-004",
      ExtensionCategory::face_pack, "presentation.face-pack.missing");
  register_and_activate(registry, missing_candidate);
  auto missing_pack = face_pack(missing_candidate); missing_pack.fallback_expression = "absent";
  assert(catalog.declare_pack(missing_pack) == PackResult::declared);
  assert(catalog.validate(missing_pack.identity.pack_id) == PackResult::rejected_missing_fallback);

  const auto no_default_candidate = asset_candidate("zie.face.nodefault", "face.nodefault", "face-008",
      ExtensionCategory::face_pack, "presentation.face-pack.nodefault");
  register_and_activate(registry, no_default_candidate);
  auto no_default = face_pack(no_default_candidate); no_default.default_expression = "absent";
  assert(catalog.declare_pack(no_default) == PackResult::declared);
  assert(catalog.validate(no_default.identity.pack_id) == PackResult::rejected_missing_default);

  auto impersonated = soft;
  impersonated.identity.pack_id = "zie.face.impersonated";
  impersonated.identity.logical_device_instance_id = "victim";
  assert(catalog.declare_pack(impersonated) == PackResult::rejected_registry_identity);

  auto executable = asset_candidate("zie.face.executable", "face.executable", "face-005",
      ExtensionCategory::face_pack, "presentation.face-pack.executable");
  executable.manifest.entrypoint = "execute_pack";
  executable.manifest.required_permissions = {Permission::write_face};
  assert(registry.register_extension(executable, {executable.manifest.id,
      TrustClass::local_developer, ControllerIdentity::none, "zie.test-profile"}) ==
      RegistryResult::rejected_invalid_manifest);

  const auto unknown_schema_candidate = asset_candidate("zie.face.schema", "face.schema", "face-006",
      ExtensionCategory::face_pack, "presentation.face-pack.schema");
  register_and_activate(registry, unknown_schema_candidate);
  auto unknown_schema = face_pack(unknown_schema_candidate); unknown_schema.identity.schema_version = {2, 0, 0};
  assert(catalog.declare_pack(unknown_schema) == PackResult::declared);
  assert(catalog.validate(unknown_schema.identity.pack_id) == PackResult::rejected_unknown_schema);

  const auto unknown_ref_candidate = asset_candidate("zie.face.reference", "face.reference", "face-007",
      ExtensionCategory::face_pack, "presentation.face-pack.reference");
  register_and_activate(registry, unknown_ref_candidate);
  auto unknown_ref = face_pack(unknown_ref_candidate); unknown_ref.transitions.front().to_expression = "absent";
  assert(catalog.declare_pack(unknown_ref) == PackResult::declared);
  assert(catalog.validate(unknown_ref.identity.pack_id) == PackResult::rejected_unknown_reference);

  auto unknown_domain = sound_pack(sound); unknown_domain.identity.domain = static_cast<PackDomain>(999);
  assert(PackCatalog(registry, 2, 1).declare_pack(unknown_domain) == PackResult::rejected_unknown_domain);
  auto unknown_kind = sound_pack(sound); unknown_kind.cues.front().kind = static_cast<SoundCueKind>(999);
  PackCatalog unknown_kind_catalog(registry, 2, 1);
  assert(unknown_kind_catalog.declare_pack(unknown_kind) == PackResult::declared);
  assert(unknown_kind_catalog.validate(unknown_kind.identity.pack_id) == PackResult::rejected_unknown_cue_kind);

  const auto client = client_candidate(); activate_client(registry, client);
  SemanticRobotApi api(registry); AuthoritativeRobotCore core; RobotStateStore state;
  assert(core.bind_command_session(api, client.manifest.id,
      client.device_identity.logical.instance_id, 50) == CommandSessionResult::bound);
  ResilientEventBus events(registry, 2, BackpressurePolicy::drop_newest);
  PresentationEngine engine(core, state, catalog, "main");
  VirtualRobot robot(api, core, state, events, &engine);
  SemanticCommand expression{SemanticCommandType::expression_intent,
      {client.manifest.id, client.device_identity.logical.instance_id, 50, 1},
      ExpressionIntent{"happy"}};
  assert(api.submit(expression) == CommandResult::accepted);
  assert(robot.execute_next() == VirtualExecutionResult::executed);
  assert(engine.current().expression == "happy");
  assert(engine.current().transition == "crossfade");
  assert(state.current(RobotStateCategory::presentation)->semantic_value == "happy");
  const auto generation = engine.current().generation;
  expression.source.session_id = 999; expression.source.sequence = 2;
  expression.payload = ExpressionIntent{"neutral"};
  assert(api.submit(expression) == CommandResult::rejected_invalid_session);
  assert(robot.execute_next() == VirtualExecutionResult::no_accepted_command);
  assert(engine.current().generation == generation);

  SemanticCommand cue{SemanticCommandType::audio_cue_intent,
      {client.manifest.id, client.device_identity.logical.instance_id, 50, 2},
      AudioCueIntent{"ready"}};
  assert(api.submit(cue) == CommandResult::accepted);
  assert(robot.execute_next() == VirtualExecutionResult::executed);
  assert(engine.current().sound_cue == "ready");
  cue.source.sequence = 3; cue.payload = AudioCueIntent{"unknown"};
  assert(api.submit(cue) == CommandResult::accepted);
  const auto before_unknown = engine.current().generation;
  assert(robot.execute_next() == VirtualExecutionResult::presentation_rejected);
  assert(engine.current().generation == before_unknown);

  assert(registry.transition(face_two.manifest.id, LifecycleState::quarantined,
                             FailureClass::security) == RegistryResult::transitioned);
  assert(catalog.active_face("main") == nullptr);
  assert(catalog.active_face("face-a") == nullptr);
  assert(catalog.active_face("face-b")->identity.pack_id ==
         soft.identity.pack_id);
  assert(registry.transition(sound.manifest.id, LifecycleState::disabled) == RegistryResult::transitioned);
  assert(catalog.active_sound("main") == nullptr);
  assert(catalog.active_sound("sound-a") == nullptr);
  assert(catalog.active_sound("sound-b") == nullptr);

  assert(registry.transition(face_two.manifest.id, LifecycleState::disabled) ==
         RegistryResult::transitioned);
  assert(registry.transition(face_two.manifest.id, LifecycleState::inactive) ==
         RegistryResult::transitioned);
  assert(registry.transition(face_two.manifest.id, LifecycleState::activating) ==
         RegistryResult::transitioned);
  assert(registry.activate_capabilities(
             face_two.manifest.id, face_two.manifest.declared_capabilities) ==
         RegistryResult::capabilities_activated);
  assert(catalog.active_face("main") == nullptr);
  assert(catalog.activate_face("main", bold.identity.pack_id) ==
         PackResult::activated);
  assert(catalog.active_face("main")->identity.pack_id == bold.identity.pack_id);

  assert(registry.transition(sound.manifest.id, LifecycleState::inactive) ==
         RegistryResult::transitioned);
  assert(registry.transition(sound.manifest.id, LifecycleState::activating) ==
         RegistryResult::transitioned);
  assert(registry.activate_capabilities(
             sound.manifest.id, sound.manifest.declared_capabilities) ==
         RegistryResult::capabilities_activated);
  assert(catalog.active_sound("main") == nullptr);
  assert(catalog.activate_sound("main", basic_sound.identity.pack_id) ==
         PackResult::activated);
}
