#include "zie/api/VirtualRobot.hpp"

#include "core/AuthoritativeRobotCore.hpp"

#include <algorithm>

namespace zie::api {
namespace {
bool known(const VirtualOperation value) {
  switch (value) {
    case VirtualOperation::motion:
    case VirtualOperation::stop:
    case VirtualOperation::presentation:
    case VirtualOperation::audio:
    case VirtualOperation::sensor_query:
      return true;
  }
  return false;
}
}  // namespace

VirtualConfigurationResult VirtualRobot::set_success(
    const VirtualOperation operation, const bool success) {
  if (!known(operation)) {
    return VirtualConfigurationResult::rejected_unknown_operation;
  }
  auto found = std::find_if(outcomes_.begin(), outcomes_.end(),
                            [operation](const Outcome& outcome) {
                              return outcome.operation == operation;
                            });
  if (found == outcomes_.end()) outcomes_.push_back({operation, success});
  else found->succeeds = success;
  return VirtualConfigurationResult::configured;
}

bool VirtualRobot::succeeds(const VirtualOperation operation) const {
  const auto found = std::find_if(outcomes_.begin(), outcomes_.end(),
                                  [operation](const Outcome& outcome) {
                                    return outcome.operation == operation;
                                  });
  return found == outcomes_.end() || found->succeeds;
}

std::uint64_t VirtualRobot::next_generation(
    const RobotStateCategory category) const {
  const auto* current = state_.current(category);
  return current == nullptr ? 1 : current->generation + 1;
}

VirtualExecutionResult VirtualRobot::execute_next() {
  auto accepted = api_.take_next_accepted();
  if (!accepted.has_value()) {
    return VirtualExecutionResult::no_accepted_command;
  }
  const auto& command = accepted->command();
  VirtualOperation operation;
  RobotStateCategory category;
  std::string state_value;
  switch (command.type) {
    case SemanticCommandType::motion_intent:
      operation = VirtualOperation::motion;
      category = RobotStateCategory::motion;
      state_value = "moving";
      break;
    case SemanticCommandType::stop_motion:
      operation = VirtualOperation::stop;
      category = RobotStateCategory::motion;
      state_value = "stopped";
      break;
    case SemanticCommandType::expression_intent:
      operation = VirtualOperation::presentation;
      category = RobotStateCategory::presentation;
      state_value = std::get<ExpressionIntent>(command.payload).expression;
      break;
    case SemanticCommandType::audio_speech_intent:
      operation = VirtualOperation::audio;
      category = RobotStateCategory::audio;
      state_value = "speaking";
      break;
    case SemanticCommandType::sensor_query:
      operation = VirtualOperation::sensor_query;
      category = RobotStateCategory::sensors;
      state_value = "query-complete";
      break;
    case SemanticCommandType::protected_safety_operation:
      return VirtualExecutionResult::rejected_unknown_command;
  }
  if (!succeeds(operation)) {
    events_.publish({next_event_id_++, EventCategory::command_rejected,
                     EventSourceType::authoritative_core, "virtual-robot",
                     "simulated-device-failure"});
    return VirtualExecutionResult::simulated_failure;
  }
  if (core_.update_state(
          state_, {category, next_generation(category), state_value}) !=
      StateResult::accepted) {
    return VirtualExecutionResult::state_update_failed;
  }
  const auto event_category = operation == VirtualOperation::sensor_query
                                  ? EventCategory::sensor_observation
                                  : EventCategory::command_accepted;
  events_.publish({next_event_id_++, event_category,
                   EventSourceType::authoritative_core, "virtual-robot",
                   state_value});
  return VirtualExecutionResult::executed;
}

}  // namespace zie::api
