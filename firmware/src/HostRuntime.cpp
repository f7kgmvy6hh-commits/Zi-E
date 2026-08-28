#include "core/HostRuntime.hpp"

#include <algorithm>
#include <cctype>
#include <limits>
#include <utility>

#include "core/AuthoritativeRobotCore.hpp"
#include "zie/api/ResilientEventBus.hpp"
#include "zie/api/VirtualRobot.hpp"
#include "zie/presentation/PresentationEngine.hpp"
#include "zie/providers/ProviderFoundation.hpp"

namespace zie::core {
namespace {

bool same_version(const extensions::ContractVersion left,
                  const extensions::ContractVersion right) {
  return left.major == right.major && left.minor == right.minor &&
         left.patch == right.patch;
}

bool valid_key(const std::string& value) {
  if (value.empty() ||
      !std::isalnum(static_cast<unsigned char>(value.front()))) {
    return false;
  }
  return std::all_of(value.begin(), value.end(), [](const unsigned char part) {
    return std::islower(part) || std::isdigit(part) || part == '.' ||
           part == '_' || part == '-';
  });
}

bool known(const RuntimeSubsystemDomain value) {
  switch (value) {
    case RuntimeSubsystemDomain::hardware_profile:
    case RuntimeSubsystemDomain::protected_safety:
    case RuntimeSubsystemDomain::extension:
    case RuntimeSubsystemDomain::provider:
    case RuntimeSubsystemDomain::event_bus:
    case RuntimeSubsystemDomain::state_store:
    case RuntimeSubsystemDomain::presentation:
    case RuntimeSubsystemDomain::configuration:
    case RuntimeSubsystemDomain::virtual_robot:
      return true;
  }
  return false;
}

bool known(const RuntimeReason value) {
  switch (value) {
    case RuntimeReason::none:
    case RuntimeReason::invalid_plan:
    case RuntimeReason::invalid_transition:
    case RuntimeReason::profile_unresolved:
    case RuntimeReason::profile_revoked:
    case RuntimeReason::protected_safety_unavailable:
    case RuntimeReason::extension_unavailable:
    case RuntimeReason::provider_unavailable:
    case RuntimeReason::event_bus_unavailable:
    case RuntimeReason::state_store_unavailable:
    case RuntimeReason::presentation_unavailable:
    case RuntimeReason::configuration_unavailable:
    case RuntimeReason::virtual_robot_unavailable:
    case RuntimeReason::checkpoint_corrupt:
    case RuntimeReason::checkpoint_unknown_version:
    case RuntimeReason::checkpoint_device_mismatch:
    case RuntimeReason::retry_limit_reached:
    case RuntimeReason::authority_revocation_failed:
    case RuntimeReason::shutdown:
      return true;
  }
  return false;
}

bool active_lifecycle(const sdk::ExtensionLifecycle value) {
  return value == sdk::ExtensionLifecycle::active;
}

}  // namespace

HostRuntime::HostRuntime(
    api::AuthoritativeRobotCore& core, HardwareProfileManager& profiles,
    ExtensionHost& extensions, providers::ProviderRouter& providers,
    api::ResilientEventBus& events, api::RobotStateStore& state,
    presentation::PackCatalog& presentation,
    extensions::TransactionalConfiguration& configuration,
    api::VirtualRobot& virtual_robot, const std::size_t max_subsystems,
    const std::size_t max_checkpoint_items,
    const std::size_t max_recovery_attempts)
    : core_(core),
      profiles_(profiles),
      extensions_(extensions),
      providers_(providers),
      events_(events),
      state_(state),
      presentation_(presentation),
      configuration_(configuration),
      virtual_robot_(virtual_robot),
      max_subsystems_(max_subsystems),
      max_checkpoint_items_(max_checkpoint_items),
      max_recovery_attempts_(max_recovery_attempts) {}

bool HostRuntime::next_generation() {
  if (runtime_generation_ == std::numeric_limits<std::uint64_t>::max()) {
    return false;
  }
  ++runtime_generation_;
  return runtime_generation_ != 0;
}

bool HostRuntime::valid_plan(const RuntimeStartupPlan& plan) const {
  const std::size_t readiness_entries =
      6 + plan.extensions.size() + plan.providers.size() +
      plan.configurations.size();
  if (max_subsystems_ == 0 || max_checkpoint_items_ == 0 ||
      max_recovery_attempts_ == 0 || !valid_key(plan.hardware_profile_id) ||
      !plan.require_protected_safety ||
      readiness_entries > max_subsystems_ ||
      plan.extensions.size() > max_checkpoint_items_ ||
      plan.providers.size() > max_checkpoint_items_ ||
      plan.configurations.size() > max_checkpoint_items_) {
    return false;
  }
  for (std::size_t index = 0; index < plan.extensions.size(); ++index) {
    const auto& extension = plan.extensions[index];
    if (!valid_key(extension.package_id) || extension.active_capabilities.empty() ||
        extension.active_capabilities.size() > max_checkpoint_items_) {
      return false;
    }
    for (const auto& capability : extension.active_capabilities) {
      if (!valid_key(capability)) return false;
    }
    for (std::size_t other = index + 1; other < plan.extensions.size(); ++other) {
      if (plan.extensions[other].package_id == extension.package_id) return false;
    }
  }
  for (const auto& provider : plan.providers) {
    if (!valid_key(provider.semantic_capability)) return false;
  }
  for (std::size_t index = 0; index < plan.providers.size(); ++index) {
    for (std::size_t other = index + 1; other < plan.providers.size(); ++other) {
      if (plan.providers[index].semantic_capability ==
          plan.providers[other].semantic_capability) {
        return false;
      }
    }
  }
  for (std::size_t index = 0; index < plan.configurations.size(); ++index) {
    const auto& configuration = plan.configurations[index];
    if (!valid_key(configuration.package_id)) return false;
    for (std::size_t other = index + 1; other < plan.configurations.size();
         ++other) {
      if (plan.configurations[other].package_id == configuration.package_id) {
        return false;
      }
    }
  }
  return plan.presentation.context.empty() ||
         valid_key(plan.presentation.context);
}

bool HostRuntime::set_readiness(
    const std::string& name, const RuntimeSubsystemDomain domain,
    const RuntimeReadinessState state, const RuntimeReason reason,
    const bool required) {
  if (!valid_key(name) || !known(domain) || !known(reason) ||
      readiness_generation_ == std::numeric_limits<std::uint64_t>::max()) {
    return false;
  }
  ++readiness_generation_;
  auto found = std::find_if(readiness_.begin(), readiness_.end(),
                            [domain, &name](const RuntimeReadinessEntry& entry) {
                              return entry.domain == domain &&
                                     entry.subsystem_name == name;
                            });
  const RuntimeReadinessEntry value{name, domain, state, reason,
                                    readiness_generation_, required};
  if (found != readiness_.end()) {
    *found = value;
    return true;
  }
  if (readiness_.size() >= max_subsystems_) return false;
  readiness_.push_back(value);
  return true;
}

bool HostRuntime::revoke_runtime_authority() {
  bool revoked = true;
  for (auto extension = active_plan_.extensions.rbegin();
       extension != active_plan_.extensions.rend(); ++extension) {
    if (active_lifecycle(extensions_.lifecycle(extension->package_id))) {
      revoked = extensions_.suspend_extension(extension->package_id) ==
                    ExtensionHostResult::suspended &&
                revoked;
    }
  }
  const auto active_profile = profiles_.active_profile();
  if (active_profile.has_value()) {
    revoked = profiles_.deactivate_profile(active_profile->identity.profile_id) ==
                  HardwareProfileResult::deactivated &&
              revoked;
  }
  const auto* motion = state_.current(api::RobotStateCategory::motion);
  if (motion != nullptr &&
      motion->generation == std::numeric_limits<std::uint64_t>::max()) {
    return false;
  }
  const std::uint64_t generation = motion == nullptr ? 1 : motion->generation + 1;
  return core_.update_state(state_, {api::RobotStateCategory::motion, generation,
                                     "stopped"}) ==
             api::StateResult::accepted &&
         revoked;
}

RuntimeResult HostRuntime::fail(const RuntimeReason reason,
                                const RuntimeSubsystemDomain domain,
                                const std::string& name) {
  set_readiness(name, domain, RuntimeReadinessState::failed, reason, true);
  const bool revoked = revoke_runtime_authority();
  lifecycle_ = RuntimeLifecycle::failed;
  if (!revoked) {
    set_readiness(name, domain, RuntimeReadinessState::failed,
                  RuntimeReason::authority_revocation_failed, true);
    return RuntimeResult::rejected_invalid_runtime;
  }
  return RuntimeResult::rejected_prerequisite;
}

RuntimeResult HostRuntime::coordinate_start(const RuntimeStartupPlan& plan,
                                            const bool recovery) {
  bool degraded = false;
  if (!recovery) {
    if (profiles_.resolve_profile(plan.hardware_profile_id) !=
        HardwareProfileResult::resolved) {
      return fail(RuntimeReason::profile_unresolved,
                  RuntimeSubsystemDomain::hardware_profile, "hardware-profile");
    }
  }
  const auto resolution = profiles_.resolution(plan.hardware_profile_id);
  if (!resolution.has_value() || !resolution->ready ||
      profiles_.activate_profile(plan.hardware_profile_id) !=
          HardwareProfileResult::activated) {
    return fail(RuntimeReason::profile_unresolved,
                RuntimeSubsystemDomain::hardware_profile, "hardware-profile");
  }
  if (!set_readiness("hardware-profile",
                     RuntimeSubsystemDomain::hardware_profile,
                     RuntimeReadinessState::ready, RuntimeReason::none, true)) {
    return fail(RuntimeReason::invalid_plan,
                RuntimeSubsystemDomain::hardware_profile, "hardware-profile");
  }
  selected_profile_generation_ = resolution->identity.generation;

  const bool protected_safety_ready =
      profiles_.active_profile_has_protected_safety();
  if (plan.require_protected_safety && !protected_safety_ready) {
    return fail(RuntimeReason::protected_safety_unavailable,
                RuntimeSubsystemDomain::protected_safety,
                "protected-safety");
  }
  set_readiness("protected-safety",
                RuntimeSubsystemDomain::protected_safety,
                protected_safety_ready ? RuntimeReadinessState::ready
                                       : RuntimeReadinessState::unavailable,
                protected_safety_ready
                    ? RuntimeReason::none
                    : RuntimeReason::protected_safety_unavailable,
                plan.require_protected_safety);

  set_readiness("event-bus", RuntimeSubsystemDomain::event_bus,
                RuntimeReadinessState::ready, RuntimeReason::none, true);
  set_readiness("state-store", RuntimeSubsystemDomain::state_store,
                RuntimeReadinessState::ready, RuntimeReason::none, true);
  set_readiness("virtual-robot", RuntimeSubsystemDomain::virtual_robot,
                RuntimeReadinessState::ready, RuntimeReason::none, true);

  for (const auto& extension : plan.extensions) {
    auto lifecycle = extensions_.lifecycle(extension.package_id);
    ExtensionHostResult result = ExtensionHostResult::rejected_lifecycle;
    if (lifecycle == sdk::ExtensionLifecycle::active) {
      result = extensions_.suspend_extension(extension.package_id);
      if (result == ExtensionHostResult::suspended) {
        lifecycle = sdk::ExtensionLifecycle::inactive;
      }
    } else if (lifecycle == sdk::ExtensionLifecycle::validated) {
      result = extensions_.initialize_extension(extension.package_id);
      if (result == ExtensionHostResult::initialized) {
        lifecycle = sdk::ExtensionLifecycle::initialized;
      }
    } else if (lifecycle == sdk::ExtensionLifecycle::failed) {
      result = extensions_.recover_extension(extension.package_id);
      if (result == ExtensionHostResult::recovered) {
        lifecycle = sdk::ExtensionLifecycle::inactive;
      }
    }
    if (lifecycle == sdk::ExtensionLifecycle::initialized ||
        lifecycle == sdk::ExtensionLifecycle::inactive) {
      result = extensions_.activate_extension(extension.package_id,
                                               extension.active_capabilities);
      lifecycle = extensions_.lifecycle(extension.package_id);
    }
    if (!active_lifecycle(lifecycle)) {
      if (extension.required) {
        return fail(RuntimeReason::extension_unavailable,
                    RuntimeSubsystemDomain::extension, extension.package_id);
      }
      degraded = true;
      set_readiness(extension.package_id, RuntimeSubsystemDomain::extension,
                    RuntimeReadinessState::degraded,
                    RuntimeReason::extension_unavailable, false);
    } else {
      const auto context = extensions_.context(extension.package_id);
      if (context == nullptr) {
        return fail(RuntimeReason::extension_unavailable,
                    RuntimeSubsystemDomain::extension, extension.package_id);
      }
      auto authority = std::find_if(
          extension_authorities_.begin(), extension_authorities_.end(),
          [&extension](const ExtensionAuthority& value) {
            return value.package_id == extension.package_id;
          });
      ExtensionAuthority current{extension.package_id,
                                 context->instance_epoch(),
                                 context->active_capabilities()};
      std::sort(current.active_capabilities.begin(),
                current.active_capabilities.end());
      if (authority == extension_authorities_.end()) {
        extension_authorities_.push_back(std::move(current));
      } else {
        *authority = std::move(current);
      }
      set_readiness(extension.package_id, RuntimeSubsystemDomain::extension,
                    RuntimeReadinessState::ready, RuntimeReason::none,
                    extension.required);
    }
  }

  for (const auto& provider : plan.providers) {
    if (!providers_.available(provider.semantic_capability)) {
      if (provider.required) {
        return fail(RuntimeReason::provider_unavailable,
                    RuntimeSubsystemDomain::provider,
                    provider.semantic_capability);
      }
      degraded = true;
      set_readiness(provider.semantic_capability,
                    RuntimeSubsystemDomain::provider,
                    RuntimeReadinessState::degraded,
                    RuntimeReason::provider_unavailable, false);
    } else {
      set_readiness(provider.semantic_capability,
                    RuntimeSubsystemDomain::provider,
                    RuntimeReadinessState::ready, RuntimeReason::none,
                    provider.required);
    }
  }

  for (const auto& configuration : plan.configurations) {
    if (!configuration_.active_authorized(configuration.package_id)) {
      if (configuration.required) {
        return fail(RuntimeReason::configuration_unavailable,
                    RuntimeSubsystemDomain::configuration,
                    configuration.package_id);
      }
      degraded = true;
      set_readiness(configuration.package_id,
                    RuntimeSubsystemDomain::configuration,
                    RuntimeReadinessState::degraded,
                    RuntimeReason::configuration_unavailable, false);
    } else {
      set_readiness(configuration.package_id,
                    RuntimeSubsystemDomain::configuration,
                    RuntimeReadinessState::ready, RuntimeReason::none,
                    configuration.required);
    }
  }

  const bool face_ready = !plan.presentation.require_face ||
                          presentation_.active_face(
                              plan.presentation.context) != nullptr;
  const bool sound_ready = !plan.presentation.require_sound ||
                           presentation_.active_sound(
                               plan.presentation.context) != nullptr;
  if (!face_ready || !sound_ready) {
    degraded = true;
    set_readiness("presentation", RuntimeSubsystemDomain::presentation,
                  RuntimeReadinessState::degraded,
                  RuntimeReason::presentation_unavailable, false);
  } else {
    set_readiness("presentation", RuntimeSubsystemDomain::presentation,
                  RuntimeReadinessState::ready, RuntimeReason::none, false);
  }

  lifecycle_ = RuntimeLifecycle::ready;
  lifecycle_ = degraded ? RuntimeLifecycle::degraded
                        : RuntimeLifecycle::running;
  return degraded ? RuntimeResult::degraded
                  : recovery ? RuntimeResult::recovered
                             : RuntimeResult::running;
}

RuntimeResult HostRuntime::start(const RuntimeStartupPlan& plan) {
  if (lifecycle_ != RuntimeLifecycle::created &&
      lifecycle_ != RuntimeLifecycle::stopped) {
    return RuntimeResult::rejected_lifecycle;
  }
  if (!valid_plan(plan)) return RuntimeResult::rejected_invalid_plan;
  if (!next_generation()) return RuntimeResult::rejected_generation_exhausted;
  lifecycle_ = RuntimeLifecycle::initializing;
  readiness_.clear();
  pending_checkpoint_.reset();
  active_plan_ = plan;
  extension_authorities_.clear();
  selected_profile_generation_ = 0;
  const auto* motion = state_.current(api::RobotStateCategory::motion);
  const std::uint64_t motion_generation =
      motion == nullptr ? 1 : motion->generation + 1;
  if (core_.update_state(state_, {api::RobotStateCategory::motion,
                                  motion_generation, "stopped"}) !=
      api::StateResult::accepted) {
    return fail(RuntimeReason::state_store_unavailable,
                RuntimeSubsystemDomain::state_store, "state-store");
  }
  return coordinate_start(plan, false);
}

RuntimeResult HostRuntime::shutdown() {
  if (lifecycle_ != RuntimeLifecycle::running &&
      lifecycle_ != RuntimeLifecycle::degraded &&
      lifecycle_ != RuntimeLifecycle::failed) {
    return RuntimeResult::rejected_lifecycle;
  }
  lifecycle_ = RuntimeLifecycle::stopping;
  if (!revoke_runtime_authority()) {
    lifecycle_ = RuntimeLifecycle::failed;
    return RuntimeResult::rejected_prerequisite;
  }
  for (auto& entry : readiness_) {
    set_readiness(entry.subsystem_name, entry.domain,
                  RuntimeReadinessState::unavailable,
                  RuntimeReason::shutdown, entry.required);
  }
  lifecycle_ = RuntimeLifecycle::stopped;
  return RuntimeResult::stopped;
}

RuntimeResult HostRuntime::refresh() {
  if (lifecycle_ != RuntimeLifecycle::running &&
      lifecycle_ != RuntimeLifecycle::degraded) {
    return RuntimeResult::rejected_lifecycle;
  }
  const auto active_profile = profiles_.active_profile();
  if (!active_profile.has_value() ||
      active_profile->identity.profile_id != active_plan_.hardware_profile_id ||
      active_profile->identity.generation != selected_profile_generation_) {
    return fail(RuntimeReason::profile_revoked,
                RuntimeSubsystemDomain::hardware_profile, "hardware-profile");
  }
  if (active_plan_.require_protected_safety &&
      !profiles_.active_profile_has_protected_safety()) {
    return fail(RuntimeReason::protected_safety_unavailable,
                RuntimeSubsystemDomain::protected_safety,
                "protected-safety");
  }
  for (const auto& extension : active_plan_.extensions) {
    if (!extension_authority_current(extension)) {
      if (extension.required) {
        return fail(RuntimeReason::extension_unavailable,
                    RuntimeSubsystemDomain::extension, extension.package_id);
      }
      lifecycle_ = RuntimeLifecycle::degraded;
      set_readiness(extension.package_id, RuntimeSubsystemDomain::extension,
                    RuntimeReadinessState::degraded,
                    RuntimeReason::extension_unavailable, false);
    }
  }
  for (const auto& provider : active_plan_.providers) {
    if (!providers_.available(provider.semantic_capability)) {
      if (provider.required) {
        return fail(RuntimeReason::provider_unavailable,
                    RuntimeSubsystemDomain::provider,
                    provider.semantic_capability);
      }
      lifecycle_ = RuntimeLifecycle::degraded;
      set_readiness(provider.semantic_capability,
                    RuntimeSubsystemDomain::provider,
                    RuntimeReadinessState::degraded,
                    RuntimeReason::provider_unavailable, false);
    }
  }
  for (const auto& configuration : active_plan_.configurations) {
    if (!configuration_.active_authorized(configuration.package_id)) {
      if (configuration.required) {
        return fail(RuntimeReason::configuration_unavailable,
                    RuntimeSubsystemDomain::configuration,
                    configuration.package_id);
      }
      lifecycle_ = RuntimeLifecycle::degraded;
      set_readiness(configuration.package_id,
                    RuntimeSubsystemDomain::configuration,
                    RuntimeReadinessState::degraded,
                    RuntimeReason::configuration_unavailable, false);
    }
  }
  return lifecycle_ == RuntimeLifecycle::degraded ? RuntimeResult::degraded
                                                  : RuntimeResult::running;
}

bool HostRuntime::extension_authority_current(
    const RuntimeExtensionIntent& intent) const {
  if (!active_lifecycle(extensions_.lifecycle(intent.package_id))) return false;
  const auto context = extensions_.context(intent.package_id);
  const auto authority = std::find_if(
      extension_authorities_.begin(), extension_authorities_.end(),
      [&intent](const ExtensionAuthority& value) {
        return value.package_id == intent.package_id;
      });
  if (context == nullptr || authority == extension_authorities_.end() ||
      context->instance_epoch() != authority->epoch) {
    return false;
  }
  auto expected = intent.active_capabilities;
  auto actual = context->active_capabilities();
  std::sort(expected.begin(), expected.end());
  std::sort(actual.begin(), actual.end());
  return expected == actual && actual == authority->active_capabilities;
}

RuntimeResult HostRuntime::contain_failure(
    const RuntimeSubsystemDomain domain, const std::string& subsystem_name,
    const RuntimeReason reason) {
  if (!known(domain) || !known(reason)) {
    return RuntimeResult::rejected_unknown_domain;
  }
  if (lifecycle_ != RuntimeLifecycle::running &&
      lifecycle_ != RuntimeLifecycle::degraded) {
    return RuntimeResult::rejected_lifecycle;
  }
  if (domain == RuntimeSubsystemDomain::extension &&
      active_lifecycle(extensions_.lifecycle(subsystem_name))) {
    extensions_.fail_extension(subsystem_name);
  }
  const bool required = failure_is_required(domain, subsystem_name);
  if (required) {
    fail(reason, domain, subsystem_name);
    return RuntimeResult::failure_contained;
  }
  set_readiness(subsystem_name, domain, RuntimeReadinessState::degraded,
                reason, false);
  lifecycle_ = RuntimeLifecycle::degraded;
  return RuntimeResult::failure_contained;
}

bool HostRuntime::failure_is_required(
    const RuntimeSubsystemDomain domain,
    const std::string& subsystem_name) const {
  switch (domain) {
    case RuntimeSubsystemDomain::hardware_profile:
    case RuntimeSubsystemDomain::protected_safety:
    case RuntimeSubsystemDomain::state_store:
    case RuntimeSubsystemDomain::virtual_robot:
      return true;
    case RuntimeSubsystemDomain::extension: {
      const auto found = std::find_if(
          active_plan_.extensions.begin(), active_plan_.extensions.end(),
          [&subsystem_name](const RuntimeExtensionIntent& intent) {
            return intent.package_id == subsystem_name;
          });
      return found == active_plan_.extensions.end() || found->required;
    }
    case RuntimeSubsystemDomain::provider: {
      const auto found = std::find_if(
          active_plan_.providers.begin(), active_plan_.providers.end(),
          [&subsystem_name](const RuntimeProviderIntent& intent) {
            return intent.semantic_capability == subsystem_name;
          });
      return found != active_plan_.providers.end() && found->required;
    }
    case RuntimeSubsystemDomain::configuration: {
      const auto found = std::find_if(
          active_plan_.configurations.begin(),
          active_plan_.configurations.end(),
          [&subsystem_name](const RuntimeConfigurationIntent& intent) {
            return intent.package_id == subsystem_name;
          });
      return found != active_plan_.configurations.end() && found->required;
    }
    case RuntimeSubsystemDomain::event_bus:
      return subsystem_name == "event-bus";
    case RuntimeSubsystemDomain::presentation:
      return false;
  }
  return true;
}

std::optional<RuntimeCheckpoint> HostRuntime::create_checkpoint() {
  if (lifecycle_ != RuntimeLifecycle::running &&
      lifecycle_ != RuntimeLifecycle::degraded) {
    return std::nullopt;
  }
  const auto profile = profiles_.active_profile();
  if (!profile.has_value()) return std::nullopt;
  RuntimeCheckpoint checkpoint;
  checkpoint.schema_version = supported_runtime_checkpoint_schema();
  checkpoint.runtime_generation = runtime_generation_;
  checkpoint.hardware_profile_id = profile->identity.profile_id;
  checkpoint.hardware_profile_revision = profile->identity.revision;
  checkpoint.extensions = active_plan_.extensions;
  checkpoint.providers = active_plan_.providers;
  checkpoint.presentation = active_plan_.presentation;
  checkpoint.require_protected_safety = active_plan_.require_protected_safety;
  for (const auto& entry : profile->entries) {
    for (const auto& device : entry.devices) {
      if (checkpoint.devices.size() >= max_checkpoint_items_) return std::nullopt;
      checkpoint.devices.push_back({entry.logical_slot, device.package_id,
                                    device.logical_device_instance_id,
                                    device.active_capability});
    }
  }
  for (const auto& intent : active_plan_.configurations) {
    const auto* record = configuration_.find(intent.package_id);
    checkpoint.configurations.push_back(
        {intent.package_id,
         record != nullptr && record->active_configuration.has_value()
             ? record->active_configuration->generation
             : 0,
         intent.required});
  }
  if (!active_plan_.presentation.context.empty()) {
    const auto* face =
        presentation_.active_face(active_plan_.presentation.context);
    const auto* sound =
        presentation_.active_sound(active_plan_.presentation.context);
    if (face != nullptr) checkpoint.face_pack_id = face->identity.pack_id;
    if (sound != nullptr) checkpoint.sound_pack_id = sound->identity.pack_id;
  }
  return checkpoint_valid(checkpoint)
             ? std::optional<RuntimeCheckpoint>{checkpoint}
             : std::nullopt;
}

bool HostRuntime::checkpoint_valid(const RuntimeCheckpoint& checkpoint) const {
  std::size_t aggregate_items = checkpoint.devices.size() +
                                checkpoint.configurations.size() +
                                checkpoint.extensions.size() +
                                checkpoint.providers.size();
  for (const auto& extension : checkpoint.extensions) {
    aggregate_items += extension.active_capabilities.size();
  }
  if (checkpoint.runtime_generation == 0 ||
      !valid_key(checkpoint.hardware_profile_id) ||
      checkpoint.hardware_profile_revision == 0 ||
      checkpoint.devices.size() > max_checkpoint_items_ ||
      checkpoint.configurations.size() > max_checkpoint_items_ ||
      checkpoint.extensions.size() > max_checkpoint_items_ ||
      checkpoint.providers.size() > max_checkpoint_items_ ||
      aggregate_items > max_checkpoint_items_) {
    return false;
  }
  for (const auto& device : checkpoint.devices) {
    if (!valid_key(device.logical_slot) || !valid_key(device.package_id) ||
        !valid_key(device.logical_device_instance_id) ||
        !valid_key(device.semantic_capability)) {
      return false;
    }
  }
  for (std::size_t index = 0; index < checkpoint.devices.size(); ++index) {
    for (std::size_t other = index + 1; other < checkpoint.devices.size();
         ++other) {
      if (checkpoint.devices[index].logical_slot ==
              checkpoint.devices[other].logical_slot &&
          checkpoint.devices[index].package_id ==
              checkpoint.devices[other].package_id &&
          checkpoint.devices[index].logical_device_instance_id ==
              checkpoint.devices[other].logical_device_instance_id &&
          checkpoint.devices[index].semantic_capability ==
              checkpoint.devices[other].semantic_capability) {
        return false;
      }
    }
  }
  return true;
}

bool HostRuntime::checkpoint_devices_match(
    const RuntimeCheckpoint& checkpoint) {
  const auto resolution = profiles_.resolution(checkpoint.hardware_profile_id);
  if (!resolution.has_value() || !resolution->ready ||
      resolution->identity.revision != checkpoint.hardware_profile_revision) {
    return false;
  }
  std::vector<RuntimeCheckpointDevice> current;
  for (const auto& entry : resolution->entries) {
    for (const auto& device : entry.devices) {
      current.push_back({entry.logical_slot, device.package_id,
                         device.logical_device_instance_id,
                         device.active_capability});
    }
  }
  auto order = [](const RuntimeCheckpointDevice& left,
                  const RuntimeCheckpointDevice& right) {
    if (left.logical_slot != right.logical_slot) {
      return left.logical_slot < right.logical_slot;
    }
    if (left.package_id != right.package_id) {
      return left.package_id < right.package_id;
    }
    if (left.logical_device_instance_id != right.logical_device_instance_id) {
      return left.logical_device_instance_id < right.logical_device_instance_id;
    }
    return left.semantic_capability < right.semantic_capability;
  };
  auto expected = checkpoint.devices;
  std::sort(current.begin(), current.end(), order);
  std::sort(expected.begin(), expected.end(), order);
  if (current.size() != expected.size()) return false;
  for (std::size_t index = 0; index < current.size(); ++index) {
    if (current[index].logical_slot != expected[index].logical_slot ||
        current[index].package_id != expected[index].package_id ||
        current[index].logical_device_instance_id !=
            expected[index].logical_device_instance_id ||
        current[index].semantic_capability !=
            expected[index].semantic_capability) {
      return false;
    }
  }
  return true;
}

RuntimeResult HostRuntime::prepare_recovery(
    const RuntimeCheckpoint& checkpoint) {
  if (lifecycle_ != RuntimeLifecycle::created &&
      lifecycle_ != RuntimeLifecycle::stopped &&
      lifecycle_ != RuntimeLifecycle::failed) {
    return RuntimeResult::rejected_lifecycle;
  }
  if (!same_version(checkpoint.schema_version,
                    supported_runtime_checkpoint_schema())) {
    return RuntimeResult::rejected_unknown_version;
  }
  if (!checkpoint_valid(checkpoint)) {
    return RuntimeResult::rejected_checkpoint;
  }
  if (recovery_attempts_ >= max_recovery_attempts_) {
    return RuntimeResult::rejected_retry_limit;
  }
  ++recovery_attempts_;
  if (!next_generation()) return RuntimeResult::rejected_generation_exhausted;
  if (runtime_generation_ <= checkpoint.runtime_generation) {
    if (checkpoint.runtime_generation ==
        std::numeric_limits<std::uint64_t>::max()) {
      return RuntimeResult::rejected_generation_exhausted;
    }
    runtime_generation_ = checkpoint.runtime_generation + 1;
  }
  lifecycle_ = RuntimeLifecycle::initializing;
  readiness_.clear();
  const auto* motion = state_.current(api::RobotStateCategory::motion);
  const std::uint64_t generation =
      motion == nullptr ? 1 : motion->generation + 1;
  if (core_.update_state(state_, {api::RobotStateCategory::motion, generation,
                                  "stopped"}) != api::StateResult::accepted) {
    lifecycle_ = RuntimeLifecycle::failed;
    return RuntimeResult::rejected_prerequisite;
  }
  if (profiles_.resolve_profile(checkpoint.hardware_profile_id) !=
          HardwareProfileResult::resolved ||
      !checkpoint_devices_match(checkpoint)) {
    lifecycle_ = RuntimeLifecycle::failed;
    set_readiness("hardware-profile",
                  RuntimeSubsystemDomain::hardware_profile,
                  RuntimeReadinessState::failed,
                  RuntimeReason::checkpoint_device_mismatch, true);
    return RuntimeResult::rejected_device_mismatch;
  }
  RuntimeStartupPlan plan;
  plan.hardware_profile_id = checkpoint.hardware_profile_id;
  plan.require_protected_safety = checkpoint.require_protected_safety;
  plan.extensions = checkpoint.extensions;
  plan.providers = checkpoint.providers;
  plan.presentation = checkpoint.presentation;
  for (const auto& configuration : checkpoint.configurations) {
    plan.configurations.push_back(
        {configuration.package_id, configuration.required});
    const auto* record = configuration_.find(configuration.package_id);
    const bool active =
        record != nullptr && record->active_configuration.has_value();
    if ((configuration.generation == 0 && active) ||
        (configuration.generation != 0 &&
         (!active || record->active_configuration->generation !=
                         configuration.generation))) {
      lifecycle_ = RuntimeLifecycle::failed;
      set_readiness(configuration.package_id,
                    RuntimeSubsystemDomain::configuration,
                    RuntimeReadinessState::failed,
                    RuntimeReason::configuration_unavailable, true);
      return RuntimeResult::rejected_checkpoint;
    }
  }
  if (!valid_plan(plan)) {
    lifecycle_ = RuntimeLifecycle::failed;
    return RuntimeResult::rejected_checkpoint;
  }
  if (!checkpoint.presentation.context.empty()) {
    const auto* face = presentation_.active_face(checkpoint.presentation.context);
    const auto* sound =
        presentation_.active_sound(checkpoint.presentation.context);
    if ((!checkpoint.face_pack_id.empty() &&
         (face == nullptr ||
          face->identity.pack_id != checkpoint.face_pack_id)) ||
        (!checkpoint.sound_pack_id.empty() &&
         (sound == nullptr ||
          sound->identity.pack_id != checkpoint.sound_pack_id))) {
      lifecycle_ = RuntimeLifecycle::failed;
      set_readiness("presentation", RuntimeSubsystemDomain::presentation,
                    RuntimeReadinessState::failed,
                    RuntimeReason::presentation_unavailable, false);
      return RuntimeResult::rejected_checkpoint;
    }
  }
  active_plan_ = std::move(plan);
  pending_checkpoint_ = checkpoint;
  return RuntimeResult::recovery_prepared;
}

RuntimeResult HostRuntime::complete_recovery() {
  if (lifecycle_ != RuntimeLifecycle::initializing ||
      !pending_checkpoint_.has_value()) {
    return RuntimeResult::rejected_lifecycle;
  }
  const auto result = coordinate_start(active_plan_, true);
  if (result == RuntimeResult::recovered ||
      result == RuntimeResult::degraded) {
    pending_checkpoint_.reset();
    recovery_attempts_ = 0;
  }
  return result;
}

RuntimeReadinessSnapshot HostRuntime::readiness() const {
  return {lifecycle_, runtime_generation_, readiness_};
}

}  // namespace zie::core
