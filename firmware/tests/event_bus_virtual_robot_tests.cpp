#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "zie/api/VirtualRobot.hpp"
#include "core/AuthoritativeRobotCore.hpp"

namespace {
using namespace zie::api;
using namespace zie::extensions;
using zie::devices::ControllerIdentity;

ExtensionCandidate candidate() {
  ExtensionCandidate value;
  value.manifest.id = "zie.virtual.client";
  value.manifest.name = "Virtual Client";
  value.manifest.version = {1, 0, 0};
  value.manifest.plugin_api_version = supported_plugin_api_version();
  value.manifest.manifest_schema_version =
      supported_manifest_schema_version();
  value.manifest.category = ExtensionCategory::integration;
  value.manifest.extension_class = ExtensionClass::host_plugin;
  value.manifest.controller_target = ControllerTarget::host;
  value.manifest.entrypoint = "create_virtual_client";
  value.manifest.declared_capabilities = {
      "semantic.motion", "semantic.presentation", "semantic.audio",
      "semantic.sensor-query", "events.read"};
  value.manifest.required_permissions = {Permission::request_motion,
                                         Permission::read_robot_state};
  value.device_identity.package.extension_id = value.manifest.id;
  value.device_identity.physical.state =
      zie::devices::PhysicalIdentityState::manufacturer_serial;
  value.device_identity.physical.manufacturer = "ZI-E Test";
  value.device_identity.physical.model = "Virtual Fixture";
  value.device_identity.physical.serial = "virtual-001";
  value.device_identity.physical.provenance = {
      zie::devices::IdentitySource::operator_verified,
      zie::devices::IdentityTrust::verified};
  value.device_identity.logical.instance_id = "virtual.client.primary";
  value.device_identity.controller.controller = ControllerIdentity::host;
  value.device_identity.hardware_profile.profile_id = "zie.test-profile";
  return value;
}

void activate(ExtensionRegistry& registry, const ExtensionCandidate& owner) {
  const std::vector<std::string> capabilities{
      "semantic.motion", "semantic.presentation", "semantic.audio",
      "semantic.sensor-query", "events.read"};
  assert(registry.register_extension(
             owner, {owner.manifest.id, TrustClass::local_developer,
                     ControllerIdentity::host, "zie.test-profile"}) ==
         RegistryResult::accepted);
  assert(registry.transition(owner.manifest.id, LifecycleState::validated) ==
         RegistryResult::transitioned);
  assert(registry.validate_capabilities(owner.manifest.id, capabilities) ==
         RegistryResult::capabilities_validated);
  assert(registry.transition(owner.manifest.id, LifecycleState::inactive) ==
         RegistryResult::transitioned);
  assert(registry.transition(owner.manifest.id, LifecycleState::activating) ==
         RegistryResult::transitioned);
  assert(registry.activate_capabilities(owner.manifest.id, capabilities) ==
         RegistryResult::capabilities_activated);
}

EventSubscriptionRequest subscription(const ExtensionCandidate& owner,
                                      const std::string& id) {
  return {{id, owner.manifest.id,
           owner.device_identity.logical.instance_id},
          "events.read",
          {EventCategory::command_accepted, EventCategory::command_rejected,
           EventCategory::sensor_observation}};
}

SemanticCommand motion(const ExtensionCandidate& owner,
                       const std::uint64_t sequence,
                       const std::uint64_t session = 100) {
  return {SemanticCommandType::motion_intent,
          {owner.manifest.id, owner.device_identity.logical.instance_id,
           session, sequence},
          MotionIntent{0.2F, 0.0F, 100}};
}
}  // namespace

void run_event_bus_virtual_robot_tests() {
  const auto owner = candidate();
  const AuthoritativeRobotCore core;
  ExtensionRegistry registry;
  activate(registry, owner);
  SemanticRobotApi api(registry);
  assert(core.bind_command_session(api, owner.manifest.id,
                                   owner.device_identity.logical.instance_id,
                                   100) == CommandSessionResult::bound);
  ResilientEventBus bus(registry, 4, BackpressurePolicy::drop_newest);
  assert(bus.subscribe(subscription(owner, "subscriber.primary")) ==
         EventBusResult::subscribed);
  assert(bus.subscribe(subscription(owner, "subscriber.primary")) ==
         EventBusResult::rejected_duplicate_subscriber);
  RobotStateStore state;
  VirtualRobot robot(api, core, state, bus);

  assert(api.submit(motion(owner, 1)) == CommandResult::accepted);
  assert(state.current(RobotStateCategory::motion) == nullptr);
  assert(robot.execute_next() == VirtualExecutionResult::executed);
  assert(state.current(RobotStateCategory::motion)->semantic_value == "moving");
  assert(bus.queued("subscriber.primary") == 1);

  auto rejected = motion(owner, 2, 999);
  assert(api.submit(rejected) == CommandResult::rejected_invalid_session);
  const auto motion_generation =
      state.current(RobotStateCategory::motion)->generation;
  assert(robot.execute_next() == VirtualExecutionResult::no_accepted_command);
  assert(state.current(RobotStateCategory::motion)->generation ==
         motion_generation);

  SemanticCommand stop{SemanticCommandType::stop_motion,
                       {owner.manifest.id,
                        owner.device_identity.logical.instance_id, 100, 2},
                       StopMotionIntent{}};
  assert(api.submit(stop) == CommandResult::accepted);
  assert(robot.execute_next() == VirtualExecutionResult::executed);
  assert(state.current(RobotStateCategory::motion)->semantic_value == "stopped");

  assert(robot.set_success(VirtualOperation::presentation, false) ==
         VirtualConfigurationResult::configured);
  SemanticCommand expression{
      SemanticCommandType::expression_intent,
      {owner.manifest.id, owner.device_identity.logical.instance_id, 100, 3},
      ExpressionIntent{"happy"}};
  assert(api.submit(expression) == CommandResult::accepted);
  assert(robot.execute_next() == VirtualExecutionResult::simulated_failure);
  assert(state.current(RobotStateCategory::presentation) == nullptr);
  assert(robot.set_success(static_cast<VirtualOperation>(999), true) ==
         VirtualConfigurationResult::rejected_unknown_operation);

  SemanticCommand sensor_query{
      SemanticCommandType::sensor_query,
      {owner.manifest.id, owner.device_identity.logical.instance_id, 100, 4},
      SensorQueryIntent{"battery-status"}};
  assert(api.submit(sensor_query) == CommandResult::accepted);
  assert(robot.execute_next() == VirtualExecutionResult::executed);
  assert(state.current(RobotStateCategory::sensors)->semantic_value ==
         "query-complete");

  ResilientEventBus isolation_bus(registry, 2,
                                  BackpressurePolicy::drop_newest, 2);
  assert(isolation_bus.subscribe(subscription(owner, "subscriber.failing")) ==
         EventBusResult::subscribed);
  assert(isolation_bus.subscribe(subscription(owner, "subscriber.healthy")) ==
         EventBusResult::subscribed);
  assert(isolation_bus.publish({10, EventCategory::command_accepted,
                                EventSourceType::authoritative_core, "core",
                                "immutable-copy"}) == EventBusResult::published);
  assert(isolation_bus.deliver_next(
             "subscriber.failing", [](const RobotEvent&) {
               throw std::runtime_error("simulated subscriber failure");
             }) == EventBusResult::subscriber_failed_retry_pending);
  assert(isolation_bus.queued("subscriber.failing") == 1);
  assert(isolation_bus.dead_lettered("subscriber.failing") == 0);
  bool healthy_received = false;
  const auto state_before_delivery =
      state.current(RobotStateCategory::motion)->generation;
  assert(isolation_bus.deliver_next(
             "subscriber.healthy", [&healthy_received](const RobotEvent& event) {
               healthy_received = event.detail == "immutable-copy";
             }) == EventBusResult::delivered);
  assert(healthy_received);
  assert(state.current(RobotStateCategory::motion)->generation ==
         state_before_delivery);
  assert(isolation_bus.deliver_next(
             "subscriber.failing", [](const RobotEvent&) {
               throw std::runtime_error("second deterministic failure");
             }) == EventBusResult::subscriber_dead_lettered);
  assert(isolation_bus.queued("subscriber.failing") == 0);
  assert(isolation_bus.dead_lettered("subscriber.failing") == 1);

  assert(isolation_bus.publish({11, EventCategory::command_accepted,
                                EventSourceType::authoritative_core, "core",
                                "retry-then-success"}) ==
         EventBusResult::published);
  assert(isolation_bus.deliver_next(
             "subscriber.failing", [](const RobotEvent&) {
               throw std::runtime_error("transient failure");
             }) == EventBusResult::subscriber_failed_retry_pending);
  std::string retried_detail;
  assert(isolation_bus.deliver_next(
             "subscriber.failing", [&retried_detail](const RobotEvent& event) {
               retried_detail = event.detail;
             }) == EventBusResult::delivered);
  assert(retried_detail == "retry-then-success");
  assert(isolation_bus.dead_lettered("subscriber.failing") == 1);

  ResilientEventBus bounded_bus(registry, 1,
                                BackpressurePolicy::drop_newest);
  assert(bounded_bus.subscribe(subscription(owner, "subscriber.bounded")) ==
         EventBusResult::subscribed);
  assert(bounded_bus.publish({20, EventCategory::command_accepted,
                              EventSourceType::authoritative_core, "core",
                              "first"}) == EventBusResult::published);
  assert(bounded_bus.publish({21, EventCategory::command_accepted,
                              EventSourceType::authoritative_core, "core",
                              "dropped-newest"}) ==
         EventBusResult::published_with_overflow);
  assert(bounded_bus.queued("subscriber.bounded") == 1);
  std::string delivered_detail;
  assert(bounded_bus.deliver_next(
             "subscriber.bounded", [&delivered_detail](const RobotEvent& event) {
               delivered_detail = event.detail;
             }) == EventBusResult::delivered);
  assert(delivered_detail == "first");

  ResilientEventBus lifecycle_bus(registry, 2,
                                  BackpressurePolicy::drop_newest);
  assert(lifecycle_bus.subscribe(subscription(owner, "subscriber.lifecycle")) ==
         EventBusResult::subscribed);
  assert(registry.transition(owner.manifest.id, LifecycleState::quarantined,
                             FailureClass::security) ==
         RegistryResult::transitioned);
  assert(lifecycle_bus.publish({30, EventCategory::command_rejected,
                                EventSourceType::authoritative_core, "core",
                                "quarantined"}) ==
         EventBusResult::published_with_ineligible_subscriber);
  assert(lifecycle_bus.deliver_next("subscriber.lifecycle",
                                    [](const RobotEvent&) {}) ==
         EventBusResult::rejected_inactive_subscriber);
  assert(registry.transition(owner.manifest.id, LifecycleState::disabled) ==
         RegistryResult::transitioned);
  assert(lifecycle_bus.publish({31, EventCategory::command_rejected,
                                EventSourceType::authoritative_core, "core",
                                "disabled"}) ==
         EventBusResult::published_with_ineligible_subscriber);
  assert(registry.unregister_extension(owner.manifest.id) ==
         RegistryResult::removed);
  assert(lifecycle_bus.publish({32, EventCategory::command_rejected,
                                EventSourceType::authoritative_core, "core",
                                "removed"}) ==
         EventBusResult::published_with_ineligible_subscriber);
  assert(lifecycle_bus.deliver_next("subscriber.lifecycle",
                                    [](const RobotEvent&) {}) ==
         EventBusResult::rejected_inactive_subscriber);

  ResilientEventBus unknown_bus(registry, 1,
                                static_cast<BackpressurePolicy>(999));
  assert(unknown_bus.subscribe(subscription(owner, "subscriber.unknown")) ==
         EventBusResult::rejected_unknown_policy);
  ResilientEventBus zero_retry_bus(registry, 1,
                                   BackpressurePolicy::drop_newest, 0);
  assert(zero_retry_bus.subscribe(subscription(owner, "subscriber.zero")) ==
         EventBusResult::rejected_invalid_bus);
  auto unknown_category = subscription(owner, "subscriber.category");
  unknown_category.categories = {static_cast<EventCategory>(999)};
  ResilientEventBus category_bus(registry, 1,
                                 BackpressurePolicy::drop_newest);
  assert(category_bus.subscribe(unknown_category) ==
         EventBusResult::rejected_unknown_category);
  assert(category_bus.publish(
             {40, static_cast<EventCategory>(999),
              EventSourceType::authoritative_core, "core", "unknown"}) ==
         EventBusResult::rejected_unknown_category);
  assert(category_bus.publish(
             {41, EventCategory::command_rejected,
              static_cast<EventSourceType>(999), "core", "unknown"}) ==
         EventBusResult::rejected_unknown_source);
  assert(bus.unsubscribe("subscriber.primary") ==
         EventBusResult::unsubscribed);
}
