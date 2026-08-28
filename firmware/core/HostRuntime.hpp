#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "core/ExtensionHost.hpp"
#include "zie/core/HardwareProfile.hpp"
#include "zie/extensions/TransactionalConfiguration.hpp"

namespace zie::api {
class AuthoritativeRobotCore;
class ResilientEventBus;
class RobotStateStore;
class VirtualRobot;
}
namespace zie::presentation { class PackCatalog; }
namespace zie::providers { class ProviderRouter; }

namespace zie::core {

enum class RuntimeLifecycle {
  created,
  initializing,
  ready,
  running,
  degraded,
  stopping,
  stopped,
  failed,
};

enum class RuntimeSubsystemDomain {
  hardware_profile,
  protected_safety,
  extension,
  provider,
  event_bus,
  state_store,
  presentation,
  configuration,
  virtual_robot,
};

enum class RuntimeReadinessState { ready, degraded, unavailable, failed };

enum class RuntimeReason {
  none,
  invalid_plan,
  invalid_transition,
  profile_unresolved,
  profile_revoked,
  protected_safety_unavailable,
  extension_unavailable,
  provider_unavailable,
  event_bus_unavailable,
  state_store_unavailable,
  presentation_unavailable,
  configuration_unavailable,
  virtual_robot_unavailable,
  checkpoint_corrupt,
  checkpoint_unknown_version,
  checkpoint_device_mismatch,
  retry_limit_reached,
  authority_revocation_failed,
  shutdown,
};

struct RuntimeReadinessEntry {
  std::string subsystem_name;
  RuntimeSubsystemDomain domain{RuntimeSubsystemDomain::hardware_profile};
  RuntimeReadinessState state{RuntimeReadinessState::unavailable};
  RuntimeReason reason{RuntimeReason::none};
  std::uint64_t generation{0};
  bool required{true};
};

struct RuntimeReadinessSnapshot {
  RuntimeLifecycle lifecycle{RuntimeLifecycle::created};
  std::uint64_t runtime_generation{0};
  std::vector<RuntimeReadinessEntry> subsystems;
};

struct RuntimeExtensionIntent {
  std::string package_id;
  std::vector<std::string> active_capabilities;
  bool required{true};
};

struct RuntimeProviderIntent {
  std::string semantic_capability;
  bool required{false};
};

struct RuntimeConfigurationIntent {
  std::string package_id;
  bool required{false};
};

struct RuntimePresentationIntent {
  std::string context;
  bool require_face{false};
  bool require_sound{false};
};

struct RuntimeStartupPlan {
  std::string hardware_profile_id;
  bool require_protected_safety{true};
  std::vector<RuntimeExtensionIntent> extensions;
  std::vector<RuntimeProviderIntent> providers;
  std::vector<RuntimeConfigurationIntent> configurations;
  RuntimePresentationIntent presentation;
};

struct RuntimeCheckpointDevice {
  std::string logical_slot;
  std::string package_id;
  std::string logical_device_instance_id;
  std::string semantic_capability;
};

struct RuntimeCheckpointConfiguration {
  std::string package_id;
  std::uint64_t generation{0};
  bool required{false};
};

struct RuntimeCheckpoint {
  extensions::ContractVersion schema_version{};
  std::uint64_t runtime_generation{0};
  std::string hardware_profile_id;
  std::uint64_t hardware_profile_revision{0};
  std::vector<RuntimeCheckpointDevice> devices;
  std::vector<RuntimeCheckpointConfiguration> configurations;
  std::vector<RuntimeExtensionIntent> extensions;
  std::vector<RuntimeProviderIntent> providers;
  RuntimePresentationIntent presentation;
  std::string face_pack_id;
  std::string sound_pack_id;
  bool require_protected_safety{true};
};

enum class RuntimeResult {
  initialized,
  running,
  degraded,
  stopped,
  checkpoint_created,
  recovery_prepared,
  recovered,
  failure_contained,
  rejected_invalid_runtime,
  rejected_invalid_plan,
  rejected_lifecycle,
  rejected_prerequisite,
  rejected_checkpoint,
  rejected_unknown_version,
  rejected_device_mismatch,
  rejected_retry_limit,
  rejected_unknown_domain,
  rejected_generation_exhausted,
};

constexpr extensions::ContractVersion supported_runtime_checkpoint_schema() {
  return {1, 0, 0};
}

class HostRuntime {
 public:
  HostRuntime(api::AuthoritativeRobotCore& core,
              HardwareProfileManager& profiles, ExtensionHost& extensions,
              providers::ProviderRouter& providers,
              api::ResilientEventBus& events, api::RobotStateStore& state,
              presentation::PackCatalog& presentation,
              extensions::TransactionalConfiguration& configuration,
              api::VirtualRobot& virtual_robot, std::size_t max_subsystems,
              std::size_t max_checkpoint_items,
              std::size_t max_recovery_attempts);

  RuntimeResult start(const RuntimeStartupPlan& plan);
  RuntimeResult shutdown();
  RuntimeResult refresh();
  RuntimeResult contain_failure(RuntimeSubsystemDomain domain,
                                const std::string& subsystem_name,
                                RuntimeReason reason);
  std::optional<RuntimeCheckpoint> create_checkpoint();
  RuntimeResult prepare_recovery(const RuntimeCheckpoint& checkpoint);
  RuntimeResult complete_recovery();

  RuntimeLifecycle lifecycle() const { return lifecycle_; }
  RuntimeReadinessSnapshot readiness() const;

 private:
  bool valid_plan(const RuntimeStartupPlan& plan) const;
  bool set_readiness(const std::string& name, RuntimeSubsystemDomain domain,
                     RuntimeReadinessState state, RuntimeReason reason,
                     bool required);
  RuntimeResult fail(RuntimeReason reason, RuntimeSubsystemDomain domain,
                     const std::string& name);
  RuntimeResult coordinate_start(const RuntimeStartupPlan& plan,
                                 bool recovery);
  bool revoke_runtime_authority();
  bool checkpoint_valid(const RuntimeCheckpoint& checkpoint) const;
  bool checkpoint_devices_match(const RuntimeCheckpoint& checkpoint);
  bool extension_authority_current(
      const RuntimeExtensionIntent& intent) const;
  bool failure_is_required(RuntimeSubsystemDomain domain,
                           const std::string& subsystem_name) const;
  bool next_generation();

  api::AuthoritativeRobotCore& core_;
  HardwareProfileManager& profiles_;
  ExtensionHost& extensions_;
  providers::ProviderRouter& providers_;
  api::ResilientEventBus& events_;
  api::RobotStateStore& state_;
  presentation::PackCatalog& presentation_;
  extensions::TransactionalConfiguration& configuration_;
  api::VirtualRobot& virtual_robot_;
  std::size_t max_subsystems_{0};
  std::size_t max_checkpoint_items_{0};
  std::size_t max_recovery_attempts_{0};
  std::size_t recovery_attempts_{0};
  std::uint64_t runtime_generation_{0};
  std::uint64_t readiness_generation_{0};
  RuntimeLifecycle lifecycle_{RuntimeLifecycle::created};
  RuntimeStartupPlan active_plan_;
  struct ExtensionAuthority {
    std::string package_id;
    std::uint64_t epoch{0};
    std::vector<std::string> active_capabilities;
  };
  std::uint64_t selected_profile_generation_{0};
  std::vector<ExtensionAuthority> extension_authorities_;
  std::vector<RuntimeReadinessEntry> readiness_;
  std::optional<RuntimeCheckpoint> pending_checkpoint_;
};

}  // namespace zie::core
