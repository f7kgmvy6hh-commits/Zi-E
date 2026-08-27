#pragma once

#include <cstdint>
#include <vector>

#include "zie/api/ResilientEventBus.hpp"

namespace zie::api {

enum class VirtualOperation { motion, stop, presentation, audio, sensor_query };
enum class VirtualConfigurationResult { configured, rejected_unknown_operation };
enum class VirtualExecutionResult {
  executed,
  simulated_failure,
  no_accepted_command,
  rejected_unknown_command,
  state_update_failed,
};

class VirtualRobot {
 public:
  VirtualRobot(SemanticRobotApi& api, const AuthoritativeRobotCore& core,
               RobotStateStore& state, ResilientEventBus& events)
      : api_(api), core_(core), state_(state), events_(events) {}

  VirtualConfigurationResult set_success(VirtualOperation operation,
                                         bool succeeds);
  VirtualExecutionResult execute_next();

 private:
  struct Outcome { VirtualOperation operation; bool succeeds{true}; };
  bool succeeds(VirtualOperation operation) const;
  std::uint64_t next_generation(RobotStateCategory category) const;
  SemanticRobotApi& api_;
  const AuthoritativeRobotCore& core_;
  RobotStateStore& state_;
  ResilientEventBus& events_;
  std::vector<Outcome> outcomes_;
  std::uint64_t next_event_id_{1};
};

}  // namespace zie::api
