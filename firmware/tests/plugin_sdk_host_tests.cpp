#include <cassert>
#include <deque>
#include <memory>
#include <type_traits>

#include "core/AuthoritativeRobotCore.hpp"
#include "core/ExtensionHost.hpp"

namespace {

using namespace zie;

class MockExtension final : public sdk::Extension {
 public:
  bool initialize(std::shared_ptr<const sdk::ExtensionContext> context) override {
    initialized_context = std::move(context);
    ++initialize_count;
    if (throw_initialize) throw 1;
    return initialize_succeeds;
  }
  bool activate(std::shared_ptr<const sdk::ExtensionContext> context) override {
    active_context = std::move(context);
    ++activate_count;
    if (throw_activate) throw 2;
    return activate_succeeds;
  }
  void suspended() override {
    ++suspend_count;
    if (throw_suspend) throw 3;
  }

  bool initialize_succeeds{true};
  bool activate_succeeds{true};
  bool throw_initialize{false};
  bool throw_activate{false};
  bool throw_suspend{false};
  std::size_t initialize_count{0};
  std::size_t activate_count{0};
  std::size_t suspend_count{0};
  std::shared_ptr<const sdk::ExtensionContext> initialized_context;
  std::shared_ptr<const sdk::ExtensionContext> active_context;
};

extensions::ExtensionCandidate candidate(
    const std::string& id, const std::vector<std::string>& capabilities,
    const extensions::ExtensionCategory category =
        extensions::ExtensionCategory::behavior,
    const extensions::ExtensionClass extension_class =
        extensions::ExtensionClass::host_plugin) {
  extensions::ExtensionCandidate value;
  value.manifest.id = id;
  value.manifest.name = "SDK mock";
  value.manifest.version = {1, 0, 0};
  value.manifest.plugin_api_version = {1, 0, 0};
  value.manifest.manifest_schema_version = {1, 0, 0};
  value.manifest.category = category;
  value.manifest.extension_class = extension_class;
  value.manifest.controller_target =
      extension_class == extensions::ExtensionClass::asset_pack
          ? extensions::ControllerTarget::none
          : extensions::ControllerTarget::host;
  value.manifest.entrypoint =
      extension_class == extensions::ExtensionClass::asset_pack
          ? std::string{}
          : "zie_sdk_create";
  value.manifest.declared_capabilities = capabilities;
  value.device_identity.package.extension_id = id;
  value.device_identity.physical.state =
      devices::PhysicalIdentityState::provisional_local;
  value.device_identity.physical.manufacturer = "zie";
  value.device_identity.physical.model = "sdk-mock";
  value.device_identity.physical.provisional_local_id = id + ".physical";
  value.device_identity.physical.provenance = {
      devices::IdentitySource::registry_generated,
      devices::IdentityTrust::provisional};
  value.device_identity.logical.instance_id = id + ".logical";
  value.device_identity.controller.controller =
      extension_class == extensions::ExtensionClass::asset_pack
          ? devices::ControllerIdentity::none
          : devices::ControllerIdentity::host;
  value.device_identity.hardware_profile.profile_id = "zie.test-profile";
  return value;
}

extensions::RegistryAssignment assignment(const std::string& id) {
  return {id, extensions::TrustClass::local_developer,
          devices::ControllerIdentity::host, "zie.test-profile"};
}

struct HostFixture {
  extensions::ExtensionRegistry registry;
  api::AuthoritativeRobotCore core;
  api::SemanticRobotApi commands{registry};
  api::ResilientEventBus events{registry, 8,
                                api::BackpressurePolicy::drop_newest};
  api::RobotStateStore state;
  extensions::TransactionalConfiguration configuration{registry};
  providers::ProviderRouter providers{registry, 8, 4, 8};
  core::ExtensionHost host{registry, core, commands, events, state,
                           configuration, providers};
};

void prepare(HostFixture& fixture,
             const extensions::ExtensionCandidate& extension_candidate,
             const std::shared_ptr<MockExtension>& extension,
             const std::vector<std::string>& capabilities) {
  assert(fixture.host.declare_extension(
             extension_candidate, assignment(extension_candidate.manifest.id),
             {{1, 0, 0}, {1, 1, 0}}, extension) ==
         core::ExtensionHostResult::declared);
  assert(fixture.host.validate_extension(extension_candidate.manifest.id,
                                         capabilities) ==
         core::ExtensionHostResult::validated);
  assert(fixture.host.initialize_extension(extension_candidate.manifest.id) ==
         core::ExtensionHostResult::initialized);
  assert(extension->initialized_context != nullptr);
  assert(extension->initialized_context->active_capabilities().empty());
  assert(extension->initialized_context->commands() == nullptr);
}

void prepare_active(HostFixture& fixture,
                    const extensions::ExtensionCandidate& extension_candidate,
                    const std::shared_ptr<MockExtension>& extension,
                    const std::vector<std::string>& capabilities) {
  prepare(fixture, extension_candidate, extension, capabilities);
  assert(fixture.host.activate_extension(extension_candidate.manifest.id,
                                         capabilities) ==
         core::ExtensionHostResult::activated);
}

}  // namespace

void run_plugin_sdk_host_tests() {
  using namespace zie;

  static_assert(std::is_abstract<sdk::ExtensionContext>::value,
                "extensions must not construct an authoritative context");
  assert(sdk::check_contract_compatibility({{1, 0, 0}, {1, 3, 0}}) ==
         sdk::ContractCompatibility::compatible);
  assert(sdk::check_contract_compatibility({{2, 0, 0}, {2, 1, 0}}) ==
         sdk::ContractCompatibility::incompatible_major);
  assert(sdk::check_contract_compatibility({{1, 1, 0}, {1, 2, 0}}) ==
         sdk::ContractCompatibility::unsupported_version);
  assert(sdk::check_contract_compatibility({{1, 1, 0}, {1, 0, 0}}) ==
         sdk::ContractCompatibility::invalid_range);

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    const auto value = candidate("zie.sdk.contract", {"semantic.presentation"});
    assert(fixture.host.declare_extension(
               value, assignment(value.manifest.id),
               {{2, 0, 0}, {2, 0, 0}}, extension) ==
           core::ExtensionHostResult::rejected_contract);
    assert(fixture.registry.find(value.manifest.id) == nullptr);
  }

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    const auto value = candidate("zie.sdk.unknown-cap", {"future.raw-domain"});
    assert(fixture.host.declare_extension(
               value, assignment(value.manifest.id),
               {{1, 0, 0}, {1, 0, 0}}, extension) ==
           core::ExtensionHostResult::declared);
    assert(fixture.host.validate_extension(value.manifest.id,
                                           {"future.raw-domain"}) ==
           core::ExtensionHostResult::rejected_capability);
  }

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    const auto value = candidate("zie.sdk.validation-retry", {"events.read"});
    assert(fixture.host.declare_extension(
               value, assignment(value.manifest.id),
               {{1, 0, 0}, {1, 0, 0}}, extension) ==
           core::ExtensionHostResult::declared);
    assert(fixture.host.validate_extension(
               value.manifest.id, {"events.read", "events.read"}) ==
           core::ExtensionHostResult::rejected_capability);
    assert(fixture.registry.find(value.manifest.id)->lifecycle ==
           extensions::LifecycleState::installed);
    assert(fixture.host.validate_extension(value.manifest.id, {"events.read"}) ==
           core::ExtensionHostResult::validated);
  }

  {
    HostFixture fixture;
    auto first = std::make_shared<MockExtension>();
    auto second = std::make_shared<MockExtension>();
    const auto first_value = candidate("zie.sdk.ambiguity-first", {"events.read"});
    const auto second_value = candidate("zie.sdk.ambiguity-second", {"events.read"});
    prepare_active(fixture, first_value, first, {"events.read"});
    prepare(fixture, second_value, second, {"events.read"});
    assert(fixture.host.activate_extension(second_value.manifest.id,
                                           {"events.read"}) ==
           core::ExtensionHostResult::rejected_capability);
    assert(fixture.registry.find(second_value.manifest.id)->lifecycle ==
           extensions::LifecycleState::inactive);
    assert(fixture.host.lifecycle(second_value.manifest.id) ==
           sdk::ExtensionLifecycle::initialized);
  }

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    const auto value = candidate("zie.sdk.wake-valid",
                                 {"provider.wake.detect"},
                                 extensions::ExtensionCategory::voice_wakeword);
    prepare_active(fixture, value, extension, {"provider.wake.detect"});
    assert(extension->active_context->commands() == nullptr);
    assert(extension->active_context->providers() == nullptr);
    assert(extension->active_context->configuration() == nullptr);
  }

  {
    HostFixture fixture;
    const auto provider_candidate = candidate(
        "zie.sdk.llm-provider", {"provider.llm.mock"},
        extensions::ExtensionCategory::ai_provider);
    assert(fixture.registry.register_extension(
               provider_candidate, assignment(provider_candidate.manifest.id)) ==
           extensions::RegistryResult::accepted);
    assert(fixture.registry.transition(provider_candidate.manifest.id,
                                       extensions::LifecycleState::validated) ==
           extensions::RegistryResult::transitioned);
    assert(fixture.registry.validate_capabilities(
               provider_candidate.manifest.id, {"provider.llm.mock"}) ==
           extensions::RegistryResult::capabilities_validated);
    assert(fixture.registry.transition(provider_candidate.manifest.id,
                                       extensions::LifecycleState::inactive) ==
           extensions::RegistryResult::transitioned);
    assert(fixture.registry.transition(provider_candidate.manifest.id,
                                       extensions::LifecycleState::activating) ==
           extensions::RegistryResult::transitioned);
    assert(fixture.registry.activate_capabilities(
               provider_candidate.manifest.id, {"provider.llm.mock"}) ==
           extensions::RegistryResult::capabilities_activated);
    auto provider = std::make_shared<providers::DeterministicMockProvider>(
        providers::ProviderKind::llm,
        std::deque<providers::ProviderCall>{{
            providers::ProviderCallStatus::success,
            providers::LlmResponse{"semantic answer"}}});
    assert(fixture.providers.add(
               {{provider_candidate.manifest.id,
                 provider_candidate.device_identity.logical.instance_id,
                 "provider.llm.mock", "provider.llm.chat"},
                provider}) == providers::ProviderResult::registered);

    auto consumer = std::make_shared<MockExtension>();
    const auto consumer_candidate = candidate(
        "zie.sdk.provider-consumer",
        {"provider.invoke.llm", "presentation.read"},
        extensions::ExtensionCategory::ai_tool);
    prepare_active(fixture, consumer_candidate, consumer,
                   consumer_candidate.manifest.declared_capabilities);
    assert(fixture.core.update_state(
               fixture.state,
               {api::RobotStateCategory::presentation, 1, "face=curious"}) ==
           api::StateResult::accepted);
    assert(consumer->active_context->providers() != nullptr);
    const auto outcome = consumer->active_context->providers()->invoke(
        {sdk::ProviderDomain::llm, "provider.llm.chat",
         sdk::LlmRequest{"hello"}});
    assert(outcome.result == sdk::CallResult::accepted);
    assert(std::get<sdk::LlmResponse>(outcome.response).text ==
           "semantic answer");
    assert(consumer->active_context->presentation()->current()->semantic_value ==
           "face=curious");
    assert(consumer->active_context->providers()->invoke(
               {static_cast<sdk::ProviderDomain>(999), "provider.llm.chat",
                sdk::LlmRequest{"unknown"}})
               .result == sdk::CallResult::rejected_unknown_domain);
    const auto old_provider_service = consumer->active_context->providers();
    assert(fixture.host.suspend_extension(consumer_candidate.manifest.id) ==
           core::ExtensionHostResult::suspended);
    assert(old_provider_service->invoke(
               {sdk::ProviderDomain::llm, "provider.llm.chat",
                sdk::LlmRequest{"stale"}})
               .result == sdk::CallResult::rejected_stale_context);
  }

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    const auto value = candidate(
        "zie.sdk.expression",
        {"semantic.presentation", "events.read", "robot.state.read",
         "configuration.read-stage", "presentation.read"});
    prepare_active(fixture, value, extension,
                   value.manifest.declared_capabilities);
    const auto context = extension->active_context;
    assert(context->package_id() == value.manifest.id);
    assert(context->logical_device_id() == value.device_identity.logical.instance_id);
    assert(context->hardware_profile_id() == "zie.test-profile");
    assert(context->trust() == sdk::TrustClass::local_developer);
    assert(context->contract_version().major == 1);
    assert(context->commands() != nullptr);
    assert(context->events() != nullptr);
    assert(context->robot_state() != nullptr);
    assert(context->configuration() != nullptr);
    assert(context->providers() == nullptr);
    assert(context->presentation() != nullptr);

    assert(context->commands()->submit(
               {sdk::CommandDomain::expression,
                sdk::ExpressionCommand{"curious"}}) ==
           sdk::CallResult::accepted);
    auto accepted = fixture.commands.take_next_accepted();
    assert(accepted.has_value());
    assert(accepted->command().source.package_id == value.manifest.id);
    assert(accepted->command().source.logical_device_instance_id ==
           value.device_identity.logical.instance_id);
    assert(accepted->command().source.session_id == context->instance_epoch());
    assert(context->commands()->submit(
               {sdk::CommandDomain::motion,
                sdk::MotionCommand{0.1F, 0.0F, 50}}) ==
           sdk::CallResult::rejected_unauthorized);
    assert(context->commands()->submit(
               {static_cast<sdk::CommandDomain>(999),
                sdk::SensorQueryCommand{"range"}}) ==
           sdk::CallResult::rejected_unknown_domain);

    assert(context->events()->subscribe(
               "observations", {sdk::EventDomain::sensor_observation}) ==
           sdk::CallResult::accepted);
    assert(fixture.events.publish(
               {1, api::EventCategory::sensor_observation,
                api::EventSourceType::authoritative_core, "core",
                "range-clear"}) == api::EventBusResult::published);
    bool received = false;
    assert(context->events()->deliver_next(
               "observations", [&received](const sdk::Event& event) {
                 received = event.detail == "range-clear";
               }) == sdk::CallResult::accepted);
    assert(received);

    assert(fixture.core.update_state(
               fixture.state,
               {api::RobotStateCategory::presentation, 1, "face=curious"}) ==
           api::StateResult::accepted);
    assert(context->robot_state()
               ->current(sdk::RobotStateDomain::presentation)
               .has_value());
    assert(context->presentation()->current().has_value());

    const auto old_commands = context->commands();
    const auto old_events = context->events();
    const auto old_state = context->robot_state();
    const auto old_configuration = context->configuration();
    const auto old_presentation = context->presentation();
    assert(fixture.host.suspend_extension(value.manifest.id) ==
           core::ExtensionHostResult::suspended);
    assert(old_commands->submit({sdk::CommandDomain::expression,
                                 sdk::ExpressionCommand{"awake"}}) ==
           sdk::CallResult::rejected_stale_context);
    assert(old_events->deliver_next("observations", [](const sdk::Event&) {}) ==
           sdk::CallResult::rejected_stale_context);
    assert(old_state->current(sdk::RobotStateDomain::presentation) ==
           std::nullopt);
    assert(old_presentation->current() == std::nullopt);
    const std::vector<sdk::ConfigurationValue> harmless_config{
        {"behavior.mode", sdk::ConfigurationDomain::semantic_text, "quiet"}};
    assert(old_configuration->stage(1, harmless_config) ==
           sdk::CallResult::rejected_stale_context);
    assert(fixture.events.deliver_next(
               value.manifest.id + "." +
                   std::to_string(context->instance_epoch()) + ".observations",
               [](const api::RobotEvent&) {}) ==
           api::EventBusResult::rejected_not_found);
  }

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    const auto value = candidate("zie.sdk.config",
                                 {"configuration.read-stage"});
    assert(fixture.host.declare_extension(
               value, assignment(value.manifest.id),
               {{1, 0, 0}, {1, 0, 0}}, extension) ==
           core::ExtensionHostResult::declared);
    const std::vector<extensions::ConfigurationDeclaration> declarations{
        {"behavior.mode",
         extensions::ConfigurationValueDomain::semantic_text,
         extensions::ConfigurationEffect::semantic_behavior, std::nullopt,
         std::nullopt}};
    assert(fixture.configuration.declare_configuration(
               value.manifest.id, declarations) ==
           extensions::ConfigurationResult::declared);
    assert(fixture.host.validate_extension(
               value.manifest.id, {"configuration.read-stage"}) ==
           core::ExtensionHostResult::validated);
    assert(fixture.host.initialize_extension(value.manifest.id) ==
           core::ExtensionHostResult::initialized);
    assert(fixture.host.activate_extension(
               value.manifest.id, {"configuration.read-stage"}) ==
           core::ExtensionHostResult::activated);
    const std::vector<sdk::ConfigurationValue> escalation{
        {"trust.class", sdk::ConfigurationDomain::semantic_text, "built_in"}};
    assert(extension->active_context->configuration()->stage(1, escalation) ==
           sdk::CallResult::accepted);
    assert(fixture.configuration.validate_staged(value.manifest.id) ==
           extensions::ConfigurationResult::rejected_authority_escalation);
    assert(fixture.configuration.commit(value.manifest.id) ==
           extensions::ConfigurationResult::rejected_not_validated);
    assert(!fixture.configuration.find(value.manifest.id)
                ->active_configuration.has_value());
  }

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    const auto value = candidate("zie.sdk.wake",
                                 {"provider.wake.detect", "semantic.motion"},
                                 extensions::ExtensionCategory::voice_wakeword);
    assert(fixture.host.declare_extension(
               value, assignment(value.manifest.id),
               {{1, 0, 0}, {1, 0, 0}}, extension) ==
           core::ExtensionHostResult::declared);
    assert(fixture.host.validate_extension(
               value.manifest.id,
               {"provider.wake.detect", "semantic.motion"}) ==
           core::ExtensionHostResult::rejected_capability);
  }

  {
    HostFixture fixture;
    auto asset = std::make_shared<MockExtension>();
    auto value = candidate("zie.sdk.asset", {},
                           extensions::ExtensionCategory::face_pack,
                           extensions::ExtensionClass::asset_pack);
    auto asset_assignment = assignment(value.manifest.id);
    asset_assignment.controller = devices::ControllerIdentity::none;
    assert(fixture.host.declare_extension(
               value, asset_assignment, {{1, 0, 0}, {1, 0, 0}}, asset) ==
           core::ExtensionHostResult::rejected_non_executable);
    assert(asset->initialized_context == nullptr);
  }

  {
    HostFixture fixture;
    auto first = std::make_shared<MockExtension>();
    const auto value = candidate("zie.sdk.epoch", {"semantic.presentation"});
    prepare_active(fixture, value, first, {"semantic.presentation"});
    const auto stale_context = first->active_context;
    assert(fixture.host.remove_extension(value.manifest.id) ==
           core::ExtensionHostResult::removed);
    assert(stale_context->commands()->submit(
               {sdk::CommandDomain::expression,
                sdk::ExpressionCommand{"stale"}}) ==
           sdk::CallResult::rejected_stale_context);

    auto same_identity = std::make_shared<MockExtension>();
    assert(fixture.host.declare_extension(
               value, assignment(value.manifest.id),
               {{1, 0, 0}, {1, 0, 0}}, same_identity) ==
           core::ExtensionHostResult::rejected_registry);

    auto replacement = std::make_shared<MockExtension>();
    auto replacement_value = value;
    replacement_value.device_identity.logical.instance_id =
        "zie.sdk.epoch.logical.replacement";
    prepare_active(fixture, replacement_value, replacement,
                   {"semantic.presentation"});
    assert(replacement->active_context->instance_epoch() !=
           stale_context->instance_epoch());
    assert(stale_context->commands()->submit(
               {sdk::CommandDomain::expression,
                sdk::ExpressionCommand{"still-stale"}}) ==
           sdk::CallResult::rejected_stale_context);
    assert(replacement->active_context->commands()->submit(
               {sdk::CommandDomain::expression,
                sdk::ExpressionCommand{"fresh"}}) ==
           sdk::CallResult::accepted);
  }

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    const auto first = candidate("zie.sdk.duplicate-a", {"events.read"});
    const auto second = candidate("zie.sdk.duplicate-b", {"events.read"});
    assert(fixture.host.declare_extension(
               first, assignment(first.manifest.id),
               {{1, 0, 0}, {1, 0, 0}}, extension) ==
           core::ExtensionHostResult::declared);
    assert(fixture.host.declare_extension(
               second, assignment(second.manifest.id),
               {{1, 0, 0}, {1, 0, 0}}, extension) ==
           core::ExtensionHostResult::rejected_duplicate_instance);
    assert(fixture.registry.find(second.manifest.id) == nullptr);
  }

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    const auto value = candidate("zie.sdk.recovery", {"semantic.presentation"});
    prepare_active(fixture, value, extension, {"semantic.presentation"});
    const auto old_commands = extension->active_context->commands();
    assert(fixture.host.fail_extension(
               value.manifest.id,
               static_cast<extensions::FailureClass>(999)) ==
           core::ExtensionHostResult::rejected_lifecycle);
    assert(old_commands->submit({sdk::CommandDomain::expression,
                                 sdk::ExpressionCommand{"still-active"}}) ==
           sdk::CallResult::accepted);
    assert(fixture.host.fail_extension(value.manifest.id) ==
           core::ExtensionHostResult::failed);
    assert(old_commands->submit({sdk::CommandDomain::expression,
                                 sdk::ExpressionCommand{"revoked"}}) ==
           sdk::CallResult::rejected_stale_context);
    assert(fixture.host.activate_extension(value.manifest.id,
                                           {"semantic.presentation"}) ==
           core::ExtensionHostResult::rejected_lifecycle);
    assert(fixture.host.recover_extension(value.manifest.id) ==
           core::ExtensionHostResult::recovered);
    assert(fixture.host.activate_extension(value.manifest.id,
                                           {"semantic.presentation"}) ==
           core::ExtensionHostResult::activated);
    assert(extension->activate_count == 2);
    assert(old_commands->submit({sdk::CommandDomain::expression,
                                 sdk::ExpressionCommand{"still-revoked"}}) ==
           sdk::CallResult::rejected_stale_context);
    const auto recovered_commands = extension->active_context->commands();
    assert(fixture.host.quarantine_extension(value.manifest.id) ==
           core::ExtensionHostResult::quarantined);
    assert(recovered_commands->submit(
               {sdk::CommandDomain::expression,
                sdk::ExpressionCommand{"quarantined"}}) ==
           sdk::CallResult::rejected_stale_context);
    assert(fixture.host.activate_extension(value.manifest.id,
                                           {"semantic.presentation"}) ==
           core::ExtensionHostResult::rejected_lifecycle);
    assert(fixture.host.recover_extension(value.manifest.id) ==
           core::ExtensionHostResult::recovered);
  }

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    extension->throw_activate = true;
    const auto value = candidate("zie.sdk.throw-activate",
                                 {"semantic.presentation"});
    prepare(fixture, value, extension, {"semantic.presentation"});
    assert(fixture.host.activate_extension(value.manifest.id,
                                           {"semantic.presentation"}) ==
           core::ExtensionHostResult::rejected_extension);
    assert(fixture.registry.find(value.manifest.id)->lifecycle ==
           extensions::LifecycleState::failed);
    assert(extension->active_context->commands()->submit(
               {sdk::CommandDomain::expression,
                sdk::ExpressionCommand{"must-not-run"}}) ==
           sdk::CallResult::rejected_stale_context);
  }

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    extension->throw_initialize = true;
    const auto value = candidate("zie.sdk.throw-initialize", {"events.read"});
    assert(fixture.host.declare_extension(
               value, assignment(value.manifest.id),
               {{1, 0, 0}, {1, 0, 0}}, extension) ==
           core::ExtensionHostResult::declared);
    assert(fixture.host.validate_extension(value.manifest.id, {"events.read"}) ==
           core::ExtensionHostResult::validated);
    assert(fixture.host.initialize_extension(value.manifest.id) ==
           core::ExtensionHostResult::rejected_extension);
    assert(fixture.registry.find(value.manifest.id)->lifecycle ==
           extensions::LifecycleState::failed);
  }

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    extension->throw_suspend = true;
    const auto value = candidate("zie.sdk.throw-suspend", {"events.read"});
    prepare_active(fixture, value, extension, {"events.read"});
    assert(fixture.host.suspend_extension(value.manifest.id) ==
           core::ExtensionHostResult::suspended);
    assert(fixture.registry.find(value.manifest.id)->lifecycle ==
           extensions::LifecycleState::inactive);
  }

  {
    HostFixture fixture;
    auto extension = std::make_shared<MockExtension>();
    const auto value = candidate("zie.sdk.external-revoke",
                                 {"semantic.presentation"});
    prepare_active(fixture, value, extension, {"semantic.presentation"});
    const auto old_commands = extension->active_context->commands();
    assert(fixture.registry.transition(value.manifest.id,
                                       extensions::LifecycleState::inactive) ==
           extensions::RegistryResult::transitioned);
    assert(fixture.registry.transition(value.manifest.id,
                                       extensions::LifecycleState::activating) ==
           extensions::RegistryResult::transitioned);
    assert(fixture.registry.activate_capabilities(
               value.manifest.id, {"semantic.presentation"}) ==
           extensions::RegistryResult::capabilities_activated);
    assert(old_commands->submit({sdk::CommandDomain::expression,
                                 sdk::ExpressionCommand{"cannot-revive"}}) ==
           sdk::CallResult::rejected_stale_context);
  }

  {
    extensions::ExtensionRegistry registry;
    api::AuthoritativeRobotCore core;
    api::SemanticRobotApi commands{registry};
    api::ResilientEventBus events{registry, 4,
                                  api::BackpressurePolicy::drop_newest};
    api::RobotStateStore state;
    extensions::TransactionalConfiguration configuration{registry};
    providers::ProviderRouter providers{registry, 4, 2, 4};
    auto host = std::make_unique<core::ExtensionHost>(
        registry, core, commands, events, state, configuration, providers);
    auto extension = std::make_shared<MockExtension>();
    const auto value = candidate("zie.sdk.host-destruction", {"events.read"});
    assert(host->declare_extension(value, assignment(value.manifest.id),
                                   {{1, 0, 0}, {1, 0, 0}}, extension) ==
           core::ExtensionHostResult::declared);
    assert(host->validate_extension(value.manifest.id, {"events.read"}) ==
           core::ExtensionHostResult::validated);
    assert(host->initialize_extension(value.manifest.id) ==
           core::ExtensionHostResult::initialized);
    assert(host->activate_extension(value.manifest.id, {"events.read"}) ==
           core::ExtensionHostResult::activated);
    const auto retained_context = extension->active_context;
    assert(retained_context->events()->subscribe(
               "retained", {sdk::EventDomain::lifecycle_changed}) ==
           sdk::CallResult::accepted);
    const auto global_subscription =
        value.manifest.id + "." +
        std::to_string(retained_context->instance_epoch()) + ".retained";
    host.reset();
    assert(retained_context->events()->deliver_next(
               "retained", [](const sdk::Event&) {}) ==
           sdk::CallResult::rejected_stale_context);
    assert(events.deliver_next(global_subscription,
                               [](const api::RobotEvent&) {}) ==
           api::EventBusResult::rejected_not_found);
  }
}
