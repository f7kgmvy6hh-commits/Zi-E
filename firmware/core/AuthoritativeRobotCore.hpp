#pragma once

#include <cstdint>
#include <string>

#include "zie/api/SemanticRobotApi.hpp"

namespace zie::api {

// Internal core authority. This header is intentionally outside the public
// include/zie API tree and must not be shipped as part of a plugin SDK.
class AuthoritativeRobotCore {
 public:
  CommandSessionResult bind_command_session(
      SemanticRobotApi& api, const std::string& package_id,
      const std::string& logical_device_instance_id,
      const std::uint64_t session_id) const {
    return api.bind_session_authoritative(package_id,
                                          logical_device_instance_id,
                                          session_id);
  }

  StateResult update_state(RobotStateStore& store,
                           const RobotStateSnapshot& snapshot) const {
    return store.update_authoritative(snapshot);
  }
};

}  // namespace zie::api
