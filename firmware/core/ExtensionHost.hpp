#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "zie/api/ResilientEventBus.hpp"
#include "zie/extensions/TransactionalConfiguration.hpp"
#include "zie/presentation/PresentationEngine.hpp"
#include "zie/providers/ProviderFoundation.hpp"
#include "zie/sdk/PluginSdk.hpp"

namespace zie::api { class AuthoritativeRobotCore; }

namespace zie::core {

class HardwareProfileManager;
class PackagePolicy;

enum class ExtensionHostResult {
  declared,
  validated,
  initialized,
  activated,
  suspended,
  failed,
  quarantined,
  removed,
  recovered,
  rejected_contract,
  rejected_non_executable,
  rejected_duplicate_instance,
  rejected_not_found,
  rejected_lifecycle,
  rejected_capability,
  rejected_registry,
  rejected_profile,
  rejected_package_policy,
  rejected_extension,
};

class ExtensionHost {
 public:
  ExtensionHost(extensions::ExtensionRegistry& registry,
                api::AuthoritativeRobotCore& core,
                api::SemanticRobotApi& commands,
                api::ResilientEventBus& events,
                api::RobotStateStore& robot_state,
                extensions::TransactionalConfiguration& configuration,
                providers::ProviderRouter& providers,
                HardwareProfileManager* hardware_profiles = nullptr,
                PackagePolicy* package_policy = nullptr);
  ~ExtensionHost();
  ExtensionHost(const ExtensionHost&) = delete;
  ExtensionHost& operator=(const ExtensionHost&) = delete;

  ExtensionHostResult declare_extension(
      const extensions::ExtensionCandidate& candidate,
      const extensions::RegistryAssignment& assignment,
      sdk::ContractRange contract,
      const std::shared_ptr<sdk::Extension>& extension);
  ExtensionHostResult validate_extension(
      const std::string& package_id,
      const std::vector<std::string>& validated_capabilities);
  ExtensionHostResult initialize_extension(const std::string& package_id);
  ExtensionHostResult activate_extension(
      const std::string& package_id,
      const std::vector<std::string>& active_capabilities);
  ExtensionHostResult suspend_extension(const std::string& package_id);
  ExtensionHostResult fail_extension(
      const std::string& package_id,
      extensions::FailureClass failure = extensions::FailureClass::temporary);
  ExtensionHostResult quarantine_extension(
      const std::string& package_id,
      extensions::FailureClass failure = extensions::FailureClass::security);
  ExtensionHostResult recover_extension(const std::string& package_id);
  ExtensionHostResult remove_extension(const std::string& package_id);

  sdk::ExtensionLifecycle lifecycle(const std::string& package_id) const;
  std::shared_ptr<const sdk::ExtensionContext> context(
      const std::string& package_id) const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace zie::core
