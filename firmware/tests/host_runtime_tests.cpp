#include <algorithm>
#include <cassert>
#include <deque>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "core/AuthoritativeRobotCore.hpp"
#include "core/HostRuntime.hpp"
#include "zie/api/VirtualRobot.hpp"

namespace {

using namespace zie;

class RuntimeExtension final : public sdk::Extension {
 public:
  bool initialize(std::shared_ptr<const sdk::ExtensionContext> context) override {
    initialized = std::move(context);
    return initialize_succeeds;
  }
  bool activate(std::shared_ptr<const sdk::ExtensionContext> context) override {
    active = std::move(context);
    return activate_succeeds;
  }
  void suspended() override { ++suspend_count; }

  bool initialize_succeeds{true};
  bool activate_succeeds{true};
  std::size_t suspend_count{0};
  std::shared_ptr<const sdk::ExtensionContext> initialized;
  std::shared_ptr<const sdk::ExtensionContext> active;
};

extensions::ExtensionCandidate drive_candidate(const std::string& id,
                                               const std::string& logical,
                                               const std::string& profile_id) {
  extensions::ExtensionCandidate value;
  value.manifest.id = id;
  value.manifest.name = "Runtime protected drive";
  value.manifest.version = {1, 0, 0};
  value.manifest.plugin_api_version = {1, 0, 0};
  value.manifest.manifest_schema_version = {1, 0, 0};
  value.manifest.category = extensions::ExtensionCategory::hardware_drive;
  value.manifest.extension_class =
      extensions::ExtensionClass::protected_safety_module;
  value.manifest.controller_target = extensions::ControllerTarget::stm32;
  value.manifest.entrypoint = "runtime_drive_entry";
  value.manifest.declared_capabilities = {"hardware.drive"};
  value.device_identity.package.extension_id = id;
  value.device_identity.physical.state =
      devices::PhysicalIdentityState::manufacturer_serial;
  value.device_identity.physical.manufacturer = "zie";
  value.device_identity.physical.model = "virtual-drive";
  value.device_identity.physical.serial = id + ".serial";
  value.device_identity.physical.provenance = {
      devices::IdentitySource::operator_verified,
      devices::IdentityTrust::verified};
  value.device_identity.logical.instance_id = logical;
  value.device_identity.controller.controller =
      devices::ControllerIdentity::stm32_safety;
  value.device_identity.hardware_profile.profile_id = profile_id;
  return value;
}

extensions::ExtensionCandidate plugin_candidate(const std::string& profile_id) {
  extensions::ExtensionCandidate value;
  value.manifest.id = "zie.runtime.client";
  value.manifest.name = "Runtime client";
  value.manifest.version = {1, 0, 0};
  value.manifest.plugin_api_version = {1, 0, 0};
  value.manifest.manifest_schema_version = {1, 0, 0};
  value.manifest.category = extensions::ExtensionCategory::integration;
  value.manifest.extension_class = extensions::ExtensionClass::host_plugin;
  value.manifest.controller_target = extensions::ControllerTarget::host;
  value.manifest.entrypoint = "runtime_client_entry";
  value.manifest.declared_capabilities = {"semantic.motion", "events.read"};
  value.manifest.required_permissions = {
      extensions::Permission::request_motion,
      extensions::Permission::read_robot_state};
  value.device_identity.package.extension_id = value.manifest.id;
  value.device_identity.physical.state =
      devices::PhysicalIdentityState::manufacturer_serial;
  value.device_identity.physical.manufacturer = "zie";
  value.device_identity.physical.model = "runtime-client";
  value.device_identity.physical.serial = "runtime-client-001";
  value.device_identity.physical.provenance = {
      devices::IdentitySource::operator_verified,
      devices::IdentityTrust::verified};
  value.device_identity.logical.instance_id = "runtime.client.primary";
  value.device_identity.controller.controller =
      devices::ControllerIdentity::host;
  value.device_identity.hardware_profile.profile_id = profile_id;
  return value;
}

extensions::ExtensionCandidate provider_candidate(
    const std::string& id, const std::string& logical,
    const std::string& capability, const std::string& profile_id) {
  auto value = plugin_candidate(profile_id);
  value.manifest.id = id;
  value.manifest.name = id;
  value.manifest.category = extensions::ExtensionCategory::ai_provider;
  value.manifest.entrypoint = "runtime_provider_entry";
  value.manifest.declared_capabilities = {capability};
  value.manifest.required_permissions.clear();
  value.device_identity.package.extension_id = id;
  value.device_identity.physical.serial = id + ".serial";
  value.device_identity.logical.instance_id = logical;
  return value;
}

extensions::RegistryAssignment assignment(
    const extensions::ExtensionCandidate& candidate,
    const extensions::TrustClass trust,
    const devices::ControllerIdentity controller) {
  return {candidate.manifest.id, trust, controller,
          candidate.device_identity.hardware_profile.profile_id, {}};
}

void register_active(extensions::ExtensionRegistry& registry,
                     const extensions::ExtensionCandidate& candidate,
                     const extensions::TrustClass trust,
                     const devices::ControllerIdentity controller) {
  assert(registry.register_extension(candidate,
                                     assignment(candidate, trust, controller)) ==
         extensions::RegistryResult::accepted);
  assert(registry.transition(candidate.manifest.id,
                             extensions::LifecycleState::validated) ==
         extensions::RegistryResult::transitioned);
  assert(registry.validate_capabilities(
             candidate.manifest.id,
             candidate.manifest.declared_capabilities) ==
         extensions::RegistryResult::capabilities_validated);
  assert(registry.transition(candidate.manifest.id,
                             extensions::LifecycleState::inactive) ==
         extensions::RegistryResult::transitioned);
  assert(registry.transition(candidate.manifest.id,
                             extensions::LifecycleState::activating) ==
         extensions::RegistryResult::transitioned);
  assert(registry.activate_capabilities(
             candidate.manifest.id,
             candidate.manifest.declared_capabilities) ==
         extensions::RegistryResult::capabilities_activated);
}

core::HardwareProfileDefinition profile(const std::string& id) {
  return {id,
          {1, 0, 0},
          1,
          core::RobotPlatformClass::zie_desktop_companion,
          {{core::HardwareDomain::drive,
            "drive.primary",
            "hardware.drive",
            extensions::ExtensionCategory::hardware_drive,
            core::ProfileRequirement::required,
            core::ProfileCardinality::exactly_one,
            core::ProfileOwnership::protected_safety_core,
            {}}}};
}

struct RuntimeFixture {
  static constexpr const char* profile_id = "zie.runtime.profile";

  RuntimeFixture()
      : commands(registry),
        events(registry, 8, api::BackpressurePolicy::drop_newest, 2),
        configuration(registry),
        providers(registry, 8, 3, 4),
        packs(registry, 8, 4),
        profiles(registry, 4, 8, 4),
        host(registry, core, commands, events, state, configuration, providers,
             &profiles),
        robot(commands, core, state, events),
        runtime(core, profiles, host, providers, events, state, packs,
                configuration, robot, 32, 16, 2) {}

  void declare_profile() {
    const auto definition = profile(profile_id);
    assert(profiles.declare_profile(
               definition,
               {profile_id, "zie.runtime-authority",
                core::ProfileSource::operator_approved}) ==
           core::HardwareProfileResult::declared);
    assert(profiles.validate_profile(profile_id) ==
           core::HardwareProfileResult::validated);
  }

  void add_drive(const extensions::ExtensionCandidate& drive) {
    register_active(registry, drive, extensions::TrustClass::built_in,
                    devices::ControllerIdentity::stm32_safety);
  }

  std::shared_ptr<RuntimeExtension> add_plugin() {
    const auto candidate = plugin_candidate(profile_id);
    auto plugin = std::make_shared<RuntimeExtension>();
    assert(host.declare_extension(
               candidate,
               assignment(candidate, extensions::TrustClass::local_developer,
                          devices::ControllerIdentity::host),
               {{1, 0, 0}, {1, 0, 0}}, plugin) ==
           core::ExtensionHostResult::declared);
    assert(host.validate_extension(candidate.manifest.id,
                                   candidate.manifest.declared_capabilities) ==
           core::ExtensionHostResult::validated);
    return plugin;
  }

  core::RuntimeStartupPlan plan(const bool with_plugin = true) const {
    core::RuntimeStartupPlan value;
    value.hardware_profile_id = profile_id;
    value.require_protected_safety = true;
    if (with_plugin) {
      value.extensions.push_back(
          {"zie.runtime.client", {"semantic.motion", "events.read"}, true});
    }
    return value;
  }

  api::AuthoritativeRobotCore core;
  extensions::ExtensionRegistry registry;
  api::SemanticRobotApi commands;
  api::ResilientEventBus events;
  api::RobotStateStore state;
  extensions::TransactionalConfiguration configuration;
  providers::ProviderRouter providers;
  presentation::PackCatalog packs;
  core::HardwareProfileManager profiles;
  core::ExtensionHost host;
  api::VirtualRobot robot;
  core::HostRuntime runtime;
};

const core::RuntimeReadinessEntry* readiness(
    const core::RuntimeReadinessSnapshot& snapshot,
    const core::RuntimeSubsystemDomain domain, const std::string& name) {
  const auto found = std::find_if(
      snapshot.subsystems.begin(), snapshot.subsystems.end(),
      [domain, &name](const core::RuntimeReadinessEntry& entry) {
        return entry.domain == domain && entry.subsystem_name == name;
      });
  return found == snapshot.subsystems.end() ? nullptr : &*found;
}

}  // namespace

void run_host_runtime_tests() {
  using namespace zie;

  {
    RuntimeFixture fixture;
    fixture.declare_profile();
    fixture.add_drive(drive_candidate("zie.runtime.drive", "drive.primary.1",
                                      RuntimeFixture::profile_id));
    const auto plugin = fixture.add_plugin();
    assert(fixture.runtime.start(fixture.plan()) == core::RuntimeResult::running);
    assert(fixture.runtime.lifecycle() == core::RuntimeLifecycle::running);
    assert(fixture.state.current(api::RobotStateCategory::motion)->semantic_value ==
           "stopped");
    assert(plugin->active != nullptr && plugin->active->commands() != nullptr);
    assert(plugin->active->events() != nullptr);
    assert(plugin->active->events()->subscribe(
               "runtime-flow", {sdk::EventDomain::command_accepted}) ==
           sdk::CallResult::accepted);
    assert(plugin->active->commands()->submit(
               {sdk::CommandDomain::motion,
                sdk::MotionCommand{0.1F, 0.0F, 100}}) ==
           sdk::CallResult::accepted);
    assert(fixture.robot.execute_next() == api::VirtualExecutionResult::executed);
    assert(fixture.state.current(api::RobotStateCategory::motion)->semantic_value ==
           "moving");
    bool event_delivered = false;
    assert(plugin->active->events()->deliver_next(
               "runtime-flow", [&event_delivered](const sdk::Event& event) {
                 event_delivered =
                     event.domain == sdk::EventDomain::command_accepted &&
                     event.detail == "moving";
               }) == sdk::CallResult::accepted);
    assert(event_delivered);

    const auto checkpoint = fixture.runtime.create_checkpoint();
    assert(checkpoint.has_value());
    const auto old_context = plugin->active;
    const auto old_epoch = old_context->instance_epoch();
    assert(fixture.runtime.shutdown() == core::RuntimeResult::stopped);
    assert(old_context->commands()->submit(
               {sdk::CommandDomain::motion,
                sdk::MotionCommand{0.1F, 0.0F, 100}}) ==
           sdk::CallResult::rejected_stale_context);
    assert(fixture.runtime.prepare_recovery(*checkpoint) ==
           core::RuntimeResult::recovery_prepared);
    assert(fixture.state.current(api::RobotStateCategory::motion)->semantic_value ==
           "stopped");
    assert(fixture.runtime.complete_recovery() ==
           core::RuntimeResult::recovered);
    assert(plugin->active->instance_epoch() != old_epoch);
    assert(old_context->commands()->submit(
               {sdk::CommandDomain::motion,
                sdk::MotionCommand{0.1F, 0.0F, 100}}) ==
           sdk::CallResult::rejected_stale_context);
  }

  {
    RuntimeFixture fixture;
    fixture.declare_profile();
    assert(fixture.runtime.start(fixture.plan(false)) ==
           core::RuntimeResult::rejected_prerequisite);
    assert(fixture.runtime.lifecycle() == core::RuntimeLifecycle::failed);
  }

  {
    RuntimeFixture fixture;
    fixture.declare_profile();
    fixture.add_drive(drive_candidate("zie.runtime.preactive-drive",
                                      "drive.preactive.1",
                                      RuntimeFixture::profile_id));
    const auto plugin = fixture.add_plugin();
    assert(fixture.profiles.resolve_profile(RuntimeFixture::profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(fixture.profiles.activate_profile(RuntimeFixture::profile_id) ==
           core::HardwareProfileResult::activated);
    assert(fixture.host.initialize_extension("zie.runtime.client") ==
           core::ExtensionHostResult::initialized);
    assert(fixture.host.activate_extension("zie.runtime.client",
                                           {"events.read"}) ==
           core::ExtensionHostResult::activated);
    const auto old_context = plugin->active;
    assert(fixture.profiles.deactivate_profile(RuntimeFixture::profile_id) ==
           core::HardwareProfileResult::deactivated);
    assert(fixture.runtime.start(fixture.plan()) == core::RuntimeResult::running);
    assert(plugin->active->instance_epoch() != old_context->instance_epoch());
    assert(plugin->active->active_capabilities().size() == 2);
    assert(old_context->events()->subscribe(
               "stale-preactive", {sdk::EventDomain::command_accepted}) ==
           sdk::CallResult::rejected_stale_context);
  }

  {
    RuntimeFixture fixture;
    fixture.declare_profile();
    fixture.add_drive(drive_candidate("zie.runtime.epoch-drive",
                                      "drive.epoch.1",
                                      RuntimeFixture::profile_id));
    fixture.add_plugin();
    assert(fixture.runtime.start(fixture.plan()) == core::RuntimeResult::running);
    assert(fixture.host.suspend_extension("zie.runtime.client") ==
           core::ExtensionHostResult::suspended);
    assert(fixture.host.activate_extension("zie.runtime.client",
                                           {"events.read"}) ==
           core::ExtensionHostResult::activated);
    assert(fixture.runtime.refresh() ==
           core::RuntimeResult::rejected_prerequisite);
    assert(fixture.runtime.lifecycle() == core::RuntimeLifecycle::failed);
  }

  {
    RuntimeFixture fixture;
    fixture.declare_profile();
    fixture.add_drive(drive_candidate("zie.runtime.policy-drive",
                                      "drive.policy.1",
                                      RuntimeFixture::profile_id));
    auto unsafe_plan = fixture.plan(false);
    unsafe_plan.require_protected_safety = false;
    assert(fixture.runtime.start(unsafe_plan) ==
           core::RuntimeResult::rejected_invalid_plan);
  }

  {
    RuntimeFixture fixture;
    fixture.declare_profile();
    fixture.add_drive(drive_candidate("zie.runtime.optional-drive",
                                      "drive.optional.1",
                                      RuntimeFixture::profile_id));
    auto plan = fixture.plan(false);
    plan.providers.push_back({"provider.llm.optional", false});
    assert(fixture.runtime.start(plan) == core::RuntimeResult::degraded);
    const auto snapshot = fixture.runtime.readiness();
    const auto* provider = readiness(snapshot, core::RuntimeSubsystemDomain::provider,
                                     "provider.llm.optional");
    assert(provider != nullptr &&
           provider->state == core::RuntimeReadinessState::degraded &&
           !provider->required);
  }

  {
    RuntimeFixture fixture;
    fixture.declare_profile();
    const auto drive = drive_candidate("zie.runtime.revoked-drive",
                                       "drive.revoked.1",
                                       RuntimeFixture::profile_id);
    fixture.add_drive(drive);
    const auto plugin = fixture.add_plugin();
    assert(fixture.runtime.start(fixture.plan()) == core::RuntimeResult::running);
    assert(fixture.registry.transition(drive.manifest.id,
                                       extensions::LifecycleState::failed,
                                       extensions::FailureClass::temporary) ==
           extensions::RegistryResult::transitioned);
    assert(fixture.runtime.refresh() ==
           core::RuntimeResult::rejected_prerequisite);
    assert(fixture.runtime.lifecycle() == core::RuntimeLifecycle::failed);
    assert(plugin->active->commands()->submit(
               {sdk::CommandDomain::motion,
                sdk::MotionCommand{0.1F, 0.0F, 100}}) ==
           sdk::CallResult::rejected_stale_context);
  }

  {
    RuntimeFixture fixture;
    fixture.declare_profile();
    fixture.add_drive(drive_candidate("zie.runtime.isolation-drive",
                                      "drive.isolation.1",
                                      RuntimeFixture::profile_id));
    fixture.add_plugin();
    auto plan = fixture.plan();
    plan.extensions.front().required = false;
    assert(fixture.runtime.start(plan) == core::RuntimeResult::running);
    assert(fixture.runtime.contain_failure(
               core::RuntimeSubsystemDomain::extension, "zie.runtime.client",
               core::RuntimeReason::extension_unavailable) ==
           core::RuntimeResult::failure_contained);
    assert(fixture.runtime.lifecycle() == core::RuntimeLifecycle::degraded);
    assert(fixture.runtime.contain_failure(
               core::RuntimeSubsystemDomain::presentation, "presentation",
               core::RuntimeReason::presentation_unavailable) ==
           core::RuntimeResult::failure_contained);
    assert(fixture.runtime.contain_failure(
               core::RuntimeSubsystemDomain::provider, "provider.llm.chat",
               core::RuntimeReason::provider_unavailable) ==
           core::RuntimeResult::failure_contained);
    assert(fixture.runtime.contain_failure(
               core::RuntimeSubsystemDomain::event_bus, "subscriber.bad",
               core::RuntimeReason::event_bus_unavailable) ==
           core::RuntimeResult::failure_contained);
    assert(fixture.runtime.contain_failure(
               core::RuntimeSubsystemDomain::configuration,
               "zie.runtime.optional-config",
               core::RuntimeReason::configuration_unavailable) ==
           core::RuntimeResult::failure_contained);
    assert(fixture.runtime.contain_failure(
               static_cast<core::RuntimeSubsystemDomain>(999), "unknown",
               core::RuntimeReason::invalid_transition) ==
           core::RuntimeResult::rejected_unknown_domain);
    assert(fixture.runtime.contain_failure(
               core::RuntimeSubsystemDomain::event_bus, "event-bus",
               core::RuntimeReason::event_bus_unavailable) ==
           core::RuntimeResult::failure_contained);
    assert(fixture.runtime.lifecycle() == core::RuntimeLifecycle::failed);
  }

  {
    RuntimeFixture fixture;
    fixture.declare_profile();
    fixture.add_drive(drive_candidate("zie.runtime.provider-drive",
                                      "drive.provider.1",
                                      RuntimeFixture::profile_id));
    const auto first = provider_candidate(
        "zie.runtime.provider-a", "provider.runtime.a", "provider.llm.a",
        RuntimeFixture::profile_id);
    const auto second = provider_candidate(
        "zie.runtime.provider-b", "provider.runtime.b", "provider.llm.b",
        RuntimeFixture::profile_id);
    register_active(fixture.registry, first,
                    extensions::TrustClass::local_developer,
                    devices::ControllerIdentity::host);
    register_active(fixture.registry, second,
                    extensions::TrustClass::local_developer,
                    devices::ControllerIdentity::host);
    auto temporary = std::make_shared<providers::DeterministicMockProvider>(
        providers::ProviderKind::llm,
        std::deque<providers::ProviderCall>{{
            providers::ProviderCallStatus::temporary_failure,
            providers::LlmResponse{}}});
    auto fallback = std::make_shared<providers::DeterministicMockProvider>(
        providers::ProviderKind::llm,
        std::deque<providers::ProviderCall>{{
            providers::ProviderCallStatus::success,
            providers::LlmResponse{"fallback-ok"}}});
    assert(fixture.providers.add(
               {{first.manifest.id, first.device_identity.logical.instance_id,
                 "provider.llm.a", "provider.llm.chat"},
                temporary}) == providers::ProviderResult::registered);
    assert(fixture.providers.add(
               {{second.manifest.id, second.device_identity.logical.instance_id,
                 "provider.llm.b", "provider.llm.chat"},
                fallback}) == providers::ProviderResult::registered);
    auto plan = fixture.plan(false);
    plan.providers.push_back({"provider.llm.chat", true});
    assert(fixture.runtime.start(plan) == core::RuntimeResult::running);
    const auto outcome = fixture.providers.invoke(
        {"provider.llm.chat", providers::LlmRequest{"hello"}});
    assert(outcome.result == providers::ProviderResult::succeeded);
    assert(outcome.provider_package_id == second.manifest.id);
    assert(fixture.runtime.refresh() == core::RuntimeResult::running);
    assert(fixture.registry.transition(first.manifest.id,
                                       extensions::LifecycleState::inactive) ==
           extensions::RegistryResult::transitioned);
    assert(fixture.registry.transition(second.manifest.id,
                                       extensions::LifecycleState::inactive) ==
           extensions::RegistryResult::transitioned);
    assert(fixture.runtime.refresh() ==
           core::RuntimeResult::rejected_prerequisite);
  }

  {
    RuntimeFixture fixture;
    fixture.declare_profile();
    fixture.add_drive(drive_candidate("zie.runtime.selected-drive",
                                      "drive.selected.1",
                                      RuntimeFixture::profile_id));
    assert(fixture.runtime.start(fixture.plan(false)) ==
           core::RuntimeResult::running);
    assert(fixture.registry.transition("zie.runtime.selected-drive",
                                       extensions::LifecycleState::inactive) ==
           extensions::RegistryResult::transitioned);
    assert(!fixture.profiles.active_profile().has_value());
    fixture.add_drive(drive_candidate("zie.runtime.substitute-drive",
                                      "drive.substitute.1",
                                      RuntimeFixture::profile_id));
    assert(fixture.profiles.resolve_profile(RuntimeFixture::profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(fixture.profiles.activate_profile(RuntimeFixture::profile_id) ==
           core::HardwareProfileResult::activated);
    assert(fixture.runtime.refresh() ==
           core::RuntimeResult::rejected_prerequisite);
    assert(fixture.runtime.lifecycle() == core::RuntimeLifecycle::failed);
  }

  {
    RuntimeFixture fixture;
    fixture.declare_profile();
    const auto first = drive_candidate("zie.runtime.original-drive",
                                       "drive.original.1",
                                       RuntimeFixture::profile_id);
    const auto replacement = drive_candidate("zie.runtime.replacement-drive",
                                             "drive.replacement.1",
                                             RuntimeFixture::profile_id);
    fixture.add_drive(first);
    assert(fixture.runtime.start(fixture.plan(false)) ==
           core::RuntimeResult::running);
    const auto checkpoint = *fixture.runtime.create_checkpoint();
    assert(fixture.runtime.shutdown() == core::RuntimeResult::stopped);
    assert(fixture.registry.transition(first.manifest.id,
                                       extensions::LifecycleState::inactive) ==
           extensions::RegistryResult::transitioned);
    fixture.add_drive(replacement);
    assert(fixture.runtime.prepare_recovery(checkpoint) ==
           core::RuntimeResult::rejected_device_mismatch);
    assert(fixture.runtime.prepare_recovery(checkpoint) ==
           core::RuntimeResult::rejected_device_mismatch);
    assert(fixture.runtime.prepare_recovery(checkpoint) ==
           core::RuntimeResult::rejected_retry_limit);
  }

  {
    RuntimeFixture fixture;
    auto corrupt = core::RuntimeCheckpoint{};
    corrupt.schema_version = {1, 0, 0};
    assert(fixture.runtime.prepare_recovery(corrupt) ==
           core::RuntimeResult::rejected_checkpoint);
    corrupt.schema_version = {2, 0, 0};
    assert(fixture.runtime.prepare_recovery(corrupt) ==
           core::RuntimeResult::rejected_unknown_version);
  }
}
