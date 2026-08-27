#include "zie/api/SemanticRobotApi.hpp"

#include <algorithm>
#include <cmath>

namespace zie::api {
namespace {

bool known(const SemanticCommandType value) {
  switch (value) {
    case SemanticCommandType::motion_intent:
    case SemanticCommandType::stop_motion:
    case SemanticCommandType::expression_intent:
    case SemanticCommandType::audio_speech_intent:
    case SemanticCommandType::audio_cue_intent:
    case SemanticCommandType::sensor_query:
    case SemanticCommandType::protected_safety_operation:
      return true;
  }
  return false;
}
bool known(const EventCategory value) {
  switch (value) {
    case EventCategory::command_accepted:
    case EventCategory::command_rejected:
    case EventCategory::capability_changed:
    case EventCategory::sensor_observation:
    case EventCategory::lifecycle_changed:
      return true;
  }
  return false;
}
bool known(const EventSourceType value) {
  switch (value) {
    case EventSourceType::authoritative_core:
    case EventSourceType::extension:
    case EventSourceType::controller:
      return true;
  }
  return false;
}
bool known(const RobotStateCategory value) {
  switch (value) {
    case RobotStateCategory::operational:
    case RobotStateCategory::motion:
    case RobotStateCategory::presentation:
    case RobotStateCategory::audio:
    case RobotStateCategory::sensors:
      return true;
  }
  return false;
}
const char* required_capability(const SemanticCommandType type) {
  switch (type) {
    case SemanticCommandType::motion_intent:
    case SemanticCommandType::stop_motion:
      return "semantic.motion";
    case SemanticCommandType::expression_intent:
      return "semantic.presentation";
    case SemanticCommandType::audio_speech_intent:
    case SemanticCommandType::audio_cue_intent:
      return "semantic.audio";
    case SemanticCommandType::sensor_query:
      return "semantic.sensor-query";
    case SemanticCommandType::protected_safety_operation:
      return nullptr;
  }
  return nullptr;
}

bool valid_payload(const SemanticCommand& command) {
  switch (command.type) {
    case SemanticCommandType::motion_intent: {
      const auto* value = std::get_if<MotionIntent>(&command.payload);
      return value != nullptr && std::isfinite(value->normalized_linear) &&
             std::isfinite(value->normalized_angular) &&
             value->normalized_linear >= -1.0F &&
             value->normalized_linear <= 1.0F &&
             value->normalized_angular >= -1.0F &&
             value->normalized_angular <= 1.0F && value->lease_ms != 0;
    }
    case SemanticCommandType::stop_motion:
      return std::holds_alternative<StopMotionIntent>(command.payload);
    case SemanticCommandType::expression_intent: {
      const auto* value = std::get_if<ExpressionIntent>(&command.payload);
      return value != nullptr && !value->expression.empty();
    }
    case SemanticCommandType::audio_speech_intent: {
      const auto* value = std::get_if<AudioSpeechIntent>(&command.payload);
      return value != nullptr && !value->utterance.empty();
    }
    case SemanticCommandType::audio_cue_intent: {
      const auto* value = std::get_if<AudioCueIntent>(&command.payload);
      return value != nullptr && !value->cue.empty();
    }
    case SemanticCommandType::sensor_query: {
      const auto* value = std::get_if<SensorQueryIntent>(&command.payload);
      return value != nullptr && !value->semantic_sensor.empty();
    }
    case SemanticCommandType::protected_safety_operation:
      return std::holds_alternative<ProtectedSafetyOperation>(command.payload);
  }
  return false;
}

}  // namespace

CommandResult SemanticRobotApi::submit(const SemanticCommand& command) {
  if (!known(command.type)) return CommandResult::rejected_unknown_command;
  if (command.type == SemanticCommandType::protected_safety_operation) {
    return CommandResult::rejected_protected_operation;
  }
  if (command.source.package_id.empty() ||
      command.source.logical_device_instance_id.empty() ||
      command.source.sequence == 0) {
    return CommandResult::rejected_invalid_identity;
  }
  const auto* record = registry_.find(command.source.package_id);
  if (record == nullptr ||
      record->device_identity.logical.instance_id !=
          command.source.logical_device_instance_id) {
    return CommandResult::rejected_invalid_identity;
  }
  if (record->lifecycle != extensions::LifecycleState::active) {
    return CommandResult::rejected_inactive_issuer;
  }
  const auto session = std::find_if(
      sessions_.begin(), sessions_.end(),
      [&command](const SessionState& state) {
        return state.active && state.package_id == command.source.package_id &&
               state.logical_device_instance_id ==
                   command.source.logical_device_instance_id;
      });
  if (command.source.session_id == 0 || session == sessions_.end() ||
      session->session_id != command.source.session_id) {
    return CommandResult::rejected_invalid_session;
  }
  const char* const capability = required_capability(command.type);
  if (capability == nullptr ||
      std::find(record->active_capabilities.begin(),
                record->active_capabilities.end(), capability) ==
          record->active_capabilities.end()) {
    return CommandResult::rejected_missing_capability;
  }
  if (!valid_payload(command)) return CommandResult::rejected_invalid_payload;

  if (command.source.sequence <= session->last_sequence) {
    return CommandResult::rejected_stale_sequence;
  }
  session->last_sequence = command.source.sequence;
  accepted_commands_.push_back(AcceptedSemanticCommand(command));
  return CommandResult::accepted;
}

CommandSessionResult SemanticRobotApi::bind_session_authoritative(
    const std::string& package_id,
    const std::string& logical_device_instance_id,
    const std::uint64_t session_id) {
  if (package_id.empty() || logical_device_instance_id.empty()) {
    return CommandSessionResult::rejected_invalid_identity;
  }
  if (session_id == 0) return CommandSessionResult::rejected_zero_session;
  const auto* record = registry_.find(package_id);
  if (record == nullptr || record->device_identity.logical.instance_id !=
                               logical_device_instance_id) {
    return CommandSessionResult::rejected_invalid_identity;
  }
  if (record->lifecycle != extensions::LifecycleState::active) {
    return CommandSessionResult::rejected_inactive_extension;
  }
  if (std::any_of(sessions_.begin(), sessions_.end(),
                  [session_id](const SessionState& state) {
                    return state.session_id == session_id;
                  })) {
    return CommandSessionResult::rejected_retired_session;
  }
  bool replaced = false;
  for (auto& state : sessions_) {
    if (state.active && state.package_id == package_id &&
        state.logical_device_instance_id == logical_device_instance_id) {
      state.active = false;
      replaced = true;
    }
  }
  sessions_.push_back(
      {package_id, logical_device_instance_id, session_id, 0, true});
  return replaced ? CommandSessionResult::replaced
                  : CommandSessionResult::bound;
}

std::optional<AcceptedSemanticCommand> SemanticRobotApi::take_next_accepted() {
  if (accepted_commands_.empty()) return std::nullopt;
  AcceptedSemanticCommand command = std::move(accepted_commands_.front());
  accepted_commands_.erase(accepted_commands_.begin());
  return command;
}

EventResult EventJournal::publish(const RobotEvent& event) {
  if (!known(event.category)) return EventResult::rejected_unknown_category;
  if (!known(event.source_type)) return EventResult::rejected_unknown_source;
  if (event.event_id == 0 || event.source_id.empty() || event.detail.empty()) {
    return EventResult::rejected_invalid_event;
  }
  events_.push_back(event);
  return EventResult::published;
}

StateResult RobotStateStore::update_authoritative(
    const RobotStateSnapshot& snapshot) {
  if (!known(snapshot.category)) return StateResult::rejected_unknown_category;
  if (snapshot.generation == 0 || snapshot.semantic_value.empty()) {
    return StateResult::rejected_invalid_state;
  }
  auto current_snapshot = std::find_if(
      snapshots_.begin(), snapshots_.end(),
      [&snapshot](const RobotStateSnapshot& value) {
        return value.category == snapshot.category;
      });
  if (current_snapshot != snapshots_.end() &&
      snapshot.generation <= current_snapshot->generation) {
    return StateResult::rejected_stale_generation;
  }
  if (current_snapshot == snapshots_.end()) {
    snapshots_.push_back(snapshot);
  } else {
    *current_snapshot = snapshot;
  }
  return StateResult::accepted;
}

const RobotStateSnapshot* RobotStateStore::current(
    const RobotStateCategory category) const {
  if (!known(category)) return nullptr;
  const auto found = std::find_if(
      snapshots_.begin(), snapshots_.end(),
      [category](const RobotStateSnapshot& value) {
        return value.category == category;
      });
  return found == snapshots_.end() ? nullptr : &*found;
}

}  // namespace zie::api
