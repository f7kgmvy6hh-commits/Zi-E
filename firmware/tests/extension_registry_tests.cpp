#include <cassert>
#include <string>
#include <utility>

#include "zie/extensions/ExtensionRegistry.hpp"

namespace {
using namespace zie::extensions;
using zie::devices::ControllerIdentity;

ExtensionManifest manifest(const std::string& id, const ExtensionClass kind,
                           const ControllerTarget target,
                           const ExtensionCategory category) {
  ExtensionManifest value;
  value.id = id;
  value.name = id;
  value.version = {1, 0, 0};
  value.plugin_api_version = supported_plugin_api_version();
  value.manifest_schema_version = supported_manifest_schema_version();
  value.extension_class = kind;
  value.controller_target = target;
  value.category = category;
  value.entrypoint = "create_extension";
  value.declared_capabilities = {"semantic.range"};
  return value;
}

ExtensionCandidate candidate(const std::string& id,
                             const ExtensionClass kind,
                             const ControllerTarget target,
                             const ControllerIdentity controller,
                             const std::string& serial,
                             const std::string& logical_id) {
  ExtensionCandidate value;
  const auto category = kind == ExtensionClass::host_plugin
                            ? ExtensionCategory::integration
                            : ExtensionCategory::hardware_range;
  value.manifest = manifest(id, kind, target, category);
  if (kind == ExtensionClass::host_plugin) {
    value.manifest.required_permissions = {Permission::request_motion};
  }
  value.device_identity.package.extension_id = id;
  value.device_identity.physical.state =
      zie::devices::PhysicalIdentityState::manufacturer_serial;
  value.device_identity.physical.manufacturer = "ZI-E Test";
  value.device_identity.physical.model = "Registry Fixture";
  value.device_identity.physical.serial = serial;
  value.device_identity.physical.provenance = {
      zie::devices::IdentitySource::operator_verified,
      zie::devices::IdentityTrust::verified};
  value.device_identity.logical.instance_id = logical_id;
  value.device_identity.controller.controller = controller;
  value.device_identity.hardware_profile.profile_id = "zie.test-profile";
  return value;
}

RegistryAssignment assignment(const std::string& id,
                              const ControllerIdentity controller,
                              const TrustClass trust) {
  return {id, trust, controller, "zie.test-profile"};
}

void validate_and_activate(ExtensionRegistry& registry, const std::string& id) {
  assert(registry.transition(id, LifecycleState::validated) ==
         RegistryResult::transitioned);
  assert(registry.validate_capabilities(id, {"semantic.range"}) ==
         RegistryResult::capabilities_validated);
  assert(registry.transition(id, LifecycleState::inactive) ==
         RegistryResult::transitioned);
  assert(registry.transition(id, LifecycleState::activating) ==
         RegistryResult::transitioned);
  assert(registry.activate_capabilities(id, {"semantic.range"}) ==
         RegistryResult::capabilities_activated);
}
}  // namespace

void run_extension_registry_tests() {
  const auto host = candidate("zie.host.range", ExtensionClass::host_plugin,
                              ControllerTarget::host, ControllerIdentity::host,
                              "host-001", "host.range.primary");
  const auto host_assignment = assignment(
      host.manifest.id, ControllerIdentity::host, TrustClass::local_developer);

  ExtensionRegistry valid_host;
  assert(valid_host.register_extension(host, host_assignment) ==
         RegistryResult::accepted);
  validate_and_activate(valid_host, host.manifest.id);
  assert(valid_host.resolve("semantic.range").size() == 1);
  assert(valid_host.resolve("semantic.range").front().package_id ==
         host.manifest.id);
  assert(valid_host.resolve("semantic.range")
             .front()
             .logical_device_instance_id == "host.range.primary");

  assert(valid_host.register_extension(host, host_assignment) ==
         RegistryResult::rejected_duplicate_package);

  auto impersonator = host;
  impersonator.manifest.id = "zie.host.victim";
  assert(ExtensionRegistry{}.register_extension(impersonator, host_assignment) ==
         RegistryResult::rejected_package_mismatch);

  auto profile_impersonator = host;
  profile_impersonator.device_identity.hardware_profile.profile_id =
      "zie.protected-profile";
  assert(ExtensionRegistry{}.register_extension(profile_impersonator,
                                                 host_assignment) ==
         RegistryResult::rejected_invalid_identity);

  const auto protected_candidate = candidate(
      "zie.safety.claim", ExtensionClass::protected_safety_module,
      ControllerTarget::stm32, ControllerIdentity::stm32_safety, "safe-001",
      "safety.claim");
  const auto untrusted_safety = assignment(protected_candidate.manifest.id,
                                           ControllerIdentity::stm32_safety,
                                           TrustClass::signed_trusted);
  assert(ExtensionRegistry{}.register_extension(protected_candidate,
                                                untrusted_safety) ==
         RegistryResult::rejected_invalid_manifest);

  const auto controller_claim_assignment = assignment(
      protected_candidate.manifest.id, ControllerIdentity::esp32_s3,
      TrustClass::built_in);
  assert(ExtensionRegistry{}.register_extension(protected_candidate,
                                                controller_claim_assignment) ==
         RegistryResult::rejected_invalid_identity);

  const auto embedded = candidate(
      "zie.embedded.range", ExtensionClass::embedded_module,
      ControllerTarget::esp32_s3, ControllerIdentity::esp32_s3, "embedded-001",
      "head.range.primary");
  const auto embedded_assignment = assignment(
      embedded.manifest.id, ControllerIdentity::esp32_s3,
      TrustClass::signed_trusted);
  ExtensionRegistry valid_embedded;
  assert(valid_embedded.register_extension(embedded, embedded_assignment) ==
         RegistryResult::accepted);
  validate_and_activate(valid_embedded, embedded.manifest.id);

  ExtensionRegistry too_early;
  assert(too_early.register_extension(host, host_assignment) ==
         RegistryResult::accepted);
  assert(too_early.activate_capabilities(host.manifest.id, {"semantic.range"}) ==
         RegistryResult::rejected_illegal_transition);
  assert(too_early.transition(host.manifest.id, LifecycleState::active) ==
         RegistryResult::rejected_illegal_transition);
  assert(too_early.resolve("semantic.range").empty());

  ExtensionRegistry unvalidated_active;
  assert(unvalidated_active.register_extension(host, host_assignment) ==
         RegistryResult::accepted);
  assert(unvalidated_active.transition(host.manifest.id,
                                       LifecycleState::validated) ==
         RegistryResult::transitioned);
  assert(unvalidated_active.validate_capabilities(host.manifest.id, {}) ==
         RegistryResult::capabilities_validated);
  assert(unvalidated_active.transition(host.manifest.id,
                                       LifecycleState::inactive) ==
         RegistryResult::transitioned);
  assert(unvalidated_active.transition(host.manifest.id,
                                       LifecycleState::activating) ==
         RegistryResult::transitioned);
  assert(unvalidated_active.transition(host.manifest.id,
                                       LifecycleState::active) ==
         RegistryResult::rejected_illegal_transition);
  assert(unvalidated_active.activate_capabilities(
             host.manifest.id, {"semantic.range"}) ==
         RegistryResult::rejected_capability_state);
  assert(unvalidated_active.find(host.manifest.id)->lifecycle ==
         LifecycleState::activating);
  assert(unvalidated_active.find(host.manifest.id)
             ->active_capabilities.empty());

  ExtensionRegistry quarantine_revocation;
  assert(quarantine_revocation.register_extension(host, host_assignment) ==
         RegistryResult::accepted);
  validate_and_activate(quarantine_revocation, host.manifest.id);
  assert(quarantine_revocation.transition(host.manifest.id,
                                          LifecycleState::quarantined,
                                          FailureClass::security) ==
         RegistryResult::transitioned);
  assert(quarantine_revocation.resolve("semantic.range").empty());

  ExtensionRegistry disable_revocation;
  assert(disable_revocation.register_extension(host, host_assignment) ==
         RegistryResult::accepted);
  validate_and_activate(disable_revocation, host.manifest.id);
  assert(disable_revocation.transition(host.manifest.id,
                                       LifecycleState::disabled) ==
         RegistryResult::transitioned);
  assert(disable_revocation.resolve("semantic.range").empty());

  ExtensionRegistry removal_revocation;
  assert(removal_revocation.register_extension(host, host_assignment) ==
         RegistryResult::accepted);
  validate_and_activate(removal_revocation, host.manifest.id);
  assert(removal_revocation.unregister_extension(host.manifest.id) ==
         RegistryResult::removed);
  assert(removal_revocation.resolve("semantic.range").empty());

  ExtensionRegistry failure_revocation;
  assert(failure_revocation.register_extension(host, host_assignment) ==
         RegistryResult::accepted);
  validate_and_activate(failure_revocation, host.manifest.id);
  assert(failure_revocation.transition(host.manifest.id,
                                       LifecycleState::failed,
                                       FailureClass::temporary) ==
         RegistryResult::transitioned);
  assert(failure_revocation.resolve("semantic.range").empty());

  ExtensionRegistry degraded_reactivation;
  assert(degraded_reactivation.register_extension(host, host_assignment) ==
         RegistryResult::accepted);
  validate_and_activate(degraded_reactivation, host.manifest.id);
  assert(degraded_reactivation.transition(host.manifest.id,
                                          LifecycleState::degraded,
                                          FailureClass::temporary) ==
         RegistryResult::transitioned);
  assert(degraded_reactivation.transition(host.manifest.id,
                                          LifecycleState::active) ==
         RegistryResult::rejected_illegal_transition);
  assert(degraded_reactivation.find(host.manifest.id)->lifecycle ==
         LifecycleState::degraded);
  assert(degraded_reactivation.transition(host.manifest.id,
                                          LifecycleState::inactive) ==
         RegistryResult::transitioned);
  assert(degraded_reactivation.transition(host.manifest.id,
                                          LifecycleState::activating) ==
         RegistryResult::transitioned);
  assert(degraded_reactivation.activate_capabilities(
             host.manifest.id, {"semantic.range"}) ==
         RegistryResult::capabilities_activated);

  ExtensionRegistry ambiguous;
  assert(ambiguous.register_extension(host, host_assignment) ==
         RegistryResult::accepted);
  auto second_host = candidate("zie.host.range-two", ExtensionClass::host_plugin,
                               ControllerTarget::host, ControllerIdentity::host,
                               "host-002", "host.range.secondary");
  const auto second_assignment = assignment(
      second_host.manifest.id, ControllerIdentity::host,
      TrustClass::local_developer);
  assert(ambiguous.register_extension(second_host, second_assignment) ==
         RegistryResult::accepted);
  validate_and_activate(ambiguous, host.manifest.id);
  assert(ambiguous.transition(second_host.manifest.id,
                              LifecycleState::validated) ==
         RegistryResult::transitioned);
  assert(ambiguous.validate_capabilities(second_host.manifest.id,
                                         {"semantic.range"}) ==
         RegistryResult::capabilities_validated);
  assert(ambiguous.transition(second_host.manifest.id,
                              LifecycleState::inactive) ==
         RegistryResult::transitioned);
  assert(ambiguous.transition(second_host.manifest.id,
                              LifecycleState::activating) ==
         RegistryResult::transitioned);
  assert(ambiguous.activate_capabilities(second_host.manifest.id,
                                         {"semantic.range"}) ==
         RegistryResult::rejected_ambiguous_capability);
  assert(ambiguous.find(second_host.manifest.id)->lifecycle ==
         LifecycleState::activating);
  assert(ambiguous.find(second_host.manifest.id)->active_capabilities.empty());

  ExtensionRegistry identity_conflict;
  assert(identity_conflict.register_extension(host, host_assignment) ==
         RegistryResult::accepted);
  auto conflicting = second_host;
  conflicting.device_identity.physical.serial = "host-001";
  assert(identity_conflict.register_extension(conflicting, second_assignment) ==
         RegistryResult::rejected_identity_conflict);

  ExtensionRegistry unknown_values;
  auto unknown_assignment = host_assignment;
  unknown_assignment.assigned_trust = static_cast<TrustClass>(999);
  assert(unknown_values.register_extension(host, unknown_assignment) ==
         RegistryResult::rejected_invalid_manifest);
  assert(unknown_values.register_extension(host, host_assignment) ==
         RegistryResult::accepted);
  assert(unknown_values.transition(host.manifest.id,
                                   static_cast<LifecycleState>(999)) ==
         RegistryResult::rejected_unknown_lifecycle);
  assert(unknown_values.transition(host.manifest.id, LifecycleState::failed,
                                   static_cast<FailureClass>(999)) ==
         RegistryResult::rejected_unknown_lifecycle);
}
