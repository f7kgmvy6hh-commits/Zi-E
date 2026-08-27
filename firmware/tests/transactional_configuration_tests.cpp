#include <cassert>
#include <string>

#include "zie/extensions/TransactionalConfiguration.hpp"

namespace {
using namespace zie::extensions;
using zie::devices::ControllerIdentity;

ExtensionCandidate host_candidate() {
  ExtensionCandidate value;
  value.manifest.id = "zie.config.host";
  value.manifest.name = "Configuration Host";
  value.manifest.version = {1, 0, 0};
  value.manifest.plugin_api_version = supported_plugin_api_version();
  value.manifest.manifest_schema_version =
      supported_manifest_schema_version();
  value.manifest.category = ExtensionCategory::integration;
  value.manifest.extension_class = ExtensionClass::host_plugin;
  value.manifest.controller_target = ControllerTarget::host;
  value.manifest.entrypoint = "create_config_host";
  value.manifest.declared_capabilities = {"semantic.configuration"};
  value.manifest.required_permissions = {Permission::read_robot_state};
  value.device_identity.package.extension_id = value.manifest.id;
  value.device_identity.physical.state =
      zie::devices::PhysicalIdentityState::manufacturer_serial;
  value.device_identity.physical.manufacturer = "ZI-E Test";
  value.device_identity.physical.model = "Configuration Fixture";
  value.device_identity.physical.serial = "config-host-001";
  value.device_identity.physical.provenance = {
      zie::devices::IdentitySource::operator_verified,
      zie::devices::IdentityTrust::verified};
  value.device_identity.logical.instance_id = "config.host.primary";
  value.device_identity.controller.controller = ControllerIdentity::host;
  value.device_identity.hardware_profile.profile_id = "zie.test-profile";
  return value;
}

RegistryAssignment assignment(const ExtensionCandidate& candidate) {
  return {candidate.manifest.id, TrustClass::local_developer,
          ControllerIdentity::host, "zie.test-profile"};
}

void activate(ExtensionRegistry& registry, const ExtensionCandidate& candidate) {
  assert(registry.register_extension(candidate, assignment(candidate)) ==
         RegistryResult::accepted);
  assert(registry.transition(candidate.manifest.id, LifecycleState::validated) ==
         RegistryResult::transitioned);
  assert(registry.validate_capabilities(candidate.manifest.id,
                                        {"semantic.configuration"}) ==
         RegistryResult::capabilities_validated);
  assert(registry.transition(candidate.manifest.id, LifecycleState::inactive) ==
         RegistryResult::transitioned);
  assert(registry.transition(candidate.manifest.id, LifecycleState::activating) ==
         RegistryResult::transitioned);
  assert(registry.activate_capabilities(candidate.manifest.id,
                                        {"semantic.configuration"}) ==
         RegistryResult::capabilities_activated);
}

ConfigurationBinding binding(const ExtensionCandidate& candidate) {
  return {candidate.manifest.id,
          candidate.device_identity.logical.instance_id,
          candidate.device_identity.hardware_profile.profile_id};
}

std::vector<ConfigurationDeclaration> declarations() {
  return {{"behavior.response-style", ConfigurationValueDomain::semantic_text,
           ConfigurationEffect::semantic_behavior},
          {"hardware.range.operating-mode",
           ConfigurationValueDomain::semantic_text,
           ConfigurationEffect::semantic_hardware}};
}

ConfigurationCandidate config(const ExtensionCandidate& owner,
                              const std::uint64_t revision,
                              std::string response_style) {
  return {binding(owner),
          revision,
          {{"behavior.response-style", ConfigurationValueDomain::semantic_text,
            std::move(response_style)},
           {"hardware.range.operating-mode",
            ConfigurationValueDomain::semantic_text, "high-accuracy"}}};
}
}  // namespace

void run_transactional_configuration_tests() {
  const auto owner = host_candidate();
  ExtensionRegistry registry;
  activate(registry, owner);
  TransactionalConfiguration configurations(registry);
  assert(configurations.declare_configuration(owner.manifest.id,
                                              declarations()) ==
         ConfigurationResult::declared);
  const ConfigurationValidationContext trusted{binding(owner)};

  auto first = config(owner, 1, "concise");
  assert(configurations.stage(first, trusted) == ConfigurationResult::staged);
  assert(configurations.validate_staged(owner.manifest.id) ==
         ConfigurationResult::validated);
  assert(configurations.commit(owner.manifest.id) ==
         ConfigurationResult::committed);
  const auto* record = configurations.find(owner.manifest.id);
  assert(record->active_configuration->revision == 1);
  assert(record->active_configuration->generation == 1);
  assert(record->active_configuration->values.front().value == "concise");

  auto invalid = config(owner, 2, "verbose");
  invalid.values.push_back({"undeclared.setting",
                            ConfigurationValueDomain::semantic_text, "bad"});
  assert(configurations.stage(invalid, trusted) == ConfigurationResult::staged);
  assert(configurations.validate_staged(owner.manifest.id) ==
         ConfigurationResult::rejected_undeclared_key);
  assert(configurations.commit(owner.manifest.id) ==
         ConfigurationResult::rejected_not_validated);
  record = configurations.find(owner.manifest.id);
  assert(record->active_configuration->revision == 1);
  assert(record->active_configuration->values.front().value == "concise");
  assert(configurations.rollback(owner.manifest.id) ==
         ConfigurationResult::rolled_back);
  assert(configurations.find(owner.manifest.id)->active_configuration->revision ==
         1);

  assert(configurations.stage(first, trusted) ==
         ConfigurationResult::rejected_stale_revision);

  auto impersonation = config(owner, 3, "impersonated");
  auto false_requester = trusted;
  false_requester.authoritative_requester.package_id = "zie.attacker";
  assert(configurations.stage(impersonation, false_requester) ==
         ConfigurationResult::rejected_identity_mismatch);
  false_requester = trusted;
  false_requester.authoritative_requester.logical_device_instance_id =
      "attacker.device";
  assert(configurations.stage(impersonation, false_requester) ==
         ConfigurationResult::rejected_identity_mismatch);

  auto escalation = config(owner, 3, "escalate");
  escalation.values.push_back(
      {"permissions.request-motion", ConfigurationValueDomain::boolean, "true"});
  assert(configurations.stage(escalation, trusted) ==
         ConfigurationResult::staged);
  assert(configurations.validate_staged(owner.manifest.id) ==
         ConfigurationResult::rejected_authority_escalation);
  assert(configurations.rollback(owner.manifest.id) ==
         ConfigurationResult::rolled_back);

  escalation = config(owner, 3, "escalate");
  escalation.values.push_back(
      {"trust.built-in", ConfigurationValueDomain::boolean, "true"});
  assert(configurations.stage(escalation, trusted) ==
         ConfigurationResult::staged);
  assert(configurations.validate_staged(owner.manifest.id) ==
         ConfigurationResult::rejected_authority_escalation);
  assert(configurations.rollback(owner.manifest.id) ==
         ConfigurationResult::rolled_back);

  escalation = config(owner, 3, "escalate");
  escalation.values.push_back(
      {"controller.stm32-safety", ConfigurationValueDomain::boolean, "true"});
  assert(configurations.stage(escalation, trusted) ==
         ConfigurationResult::staged);
  assert(configurations.validate_staged(owner.manifest.id) ==
         ConfigurationResult::rejected_authority_escalation);
  assert(configurations.rollback(owner.manifest.id) ==
         ConfigurationResult::rolled_back);

  escalation = config(owner, 3, "escalate");
  escalation.values.push_back(
      {"capabilities.raw-actuator", ConfigurationValueDomain::boolean, "true"});
  assert(configurations.stage(escalation, trusted) ==
         ConfigurationResult::staged);
  assert(configurations.validate_staged(owner.manifest.id) ==
         ConfigurationResult::rejected_authority_escalation);
  assert(configurations.rollback(owner.manifest.id) ==
         ConfigurationResult::rolled_back);

  auto unknown = config(owner, 3, "unknown");
  unknown.values.front().domain = static_cast<ConfigurationValueDomain>(999);
  assert(configurations.stage(unknown, trusted) == ConfigurationResult::staged);
  assert(configurations.validate_staged(owner.manifest.id) ==
         ConfigurationResult::rejected_unknown_domain);
  assert(configurations.rollback(owner.manifest.id) ==
         ConfigurationResult::rolled_back);

  auto update = config(owner, 3, "updated");
  assert(configurations.stage(update, trusted) == ConfigurationResult::staged);
  assert(configurations.validate_staged(owner.manifest.id) ==
         ConfigurationResult::validated);
  assert(registry.transition(owner.manifest.id, LifecycleState::quarantined,
                             FailureClass::security) ==
         RegistryResult::transitioned);
  assert(configurations.commit(owner.manifest.id) ==
         ConfigurationResult::rejected_lifecycle);
  assert(configurations.find(owner.manifest.id)->active_configuration->revision ==
         1);
  assert(registry.transition(owner.manifest.id, LifecycleState::disabled) ==
         RegistryResult::transitioned);
  assert(configurations.commit(owner.manifest.id) ==
         ConfigurationResult::rejected_lifecycle);
  assert(registry.unregister_extension(owner.manifest.id) ==
         RegistryResult::removed);
  assert(configurations.commit(owner.manifest.id) ==
         ConfigurationResult::rejected_lifecycle);
  assert(configurations.find(owner.manifest.id)->active_configuration->revision ==
         1);

  ExtensionRegistry declaration_registry;
  activate(declaration_registry, owner);
  TransactionalConfiguration invalid_declaration(declaration_registry);
  auto unknown_declaration = declarations();
  unknown_declaration.front().effect = static_cast<ConfigurationEffect>(999);
  assert(invalid_declaration.declare_configuration(owner.manifest.id,
                                                   unknown_declaration) ==
         ConfigurationResult::rejected_unknown_domain);

  TransactionalConfiguration forbidden_declaration(declaration_registry);
  const std::vector<ConfigurationDeclaration> raw_declaration{
      {"gpio.pin", ConfigurationValueDomain::bounded_integer,
       ConfigurationEffect::semantic_hardware}};
  assert(forbidden_declaration.declare_configuration(owner.manifest.id,
                                                     raw_declaration) ==
         ConfigurationResult::rejected_authority_escalation);
}
