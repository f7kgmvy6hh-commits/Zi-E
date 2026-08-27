#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "zie/extensions/ExtensionRegistry.hpp"

namespace zie::api {

enum class SemanticCommandType {
  motion_intent,
  stop_motion,
  expression_intent,
  audio_speech_intent,
  audio_cue_intent,
  sensor_query,
  protected_safety_operation,
};

struct MotionIntent {
  float normalized_linear{0.0F};
  float normalized_angular{0.0F};
  std::uint32_t lease_ms{0};
};
struct StopMotionIntent {};
struct ExpressionIntent { std::string expression; };
struct AudioSpeechIntent { std::string utterance; };
struct AudioCueIntent { std::string cue; };
struct SensorQueryIntent { std::string semantic_sensor; };
struct ProtectedSafetyOperation { std::string operation; };

using SemanticCommandPayload =
    std::variant<MotionIntent, StopMotionIntent, ExpressionIntent,
                 AudioSpeechIntent, AudioCueIntent, SensorQueryIntent,
                 ProtectedSafetyOperation>;

struct CommandSourceIdentity {
  std::string package_id;
  std::string logical_device_instance_id;
  std::uint64_t session_id{0};
  std::uint64_t sequence{0};
};

struct SemanticCommand {
  SemanticCommandType type{SemanticCommandType::sensor_query};
  CommandSourceIdentity source;
  SemanticCommandPayload payload{SensorQueryIntent{}};
};

enum class CommandResult {
  accepted,
  rejected_unknown_command,
  rejected_invalid_payload,
  rejected_invalid_identity,
  rejected_invalid_session,
  rejected_inactive_issuer,
  rejected_missing_capability,
  rejected_stale_sequence,
  rejected_protected_operation,
};

enum class CommandSessionResult {
  bound,
  replaced,
  rejected_invalid_identity,
  rejected_inactive_extension,
  rejected_zero_session,
  rejected_retired_session,
};

class AcceptedSemanticCommand {
 public:
  const SemanticCommand& command() const { return command_; }

 private:
  friend class SemanticRobotApi;
  explicit AcceptedSemanticCommand(const SemanticCommand& command)
      : command_(command) {}
  SemanticCommand command_;
};

class AuthoritativeRobotCore;

class SemanticRobotApi {
 public:
  explicit SemanticRobotApi(const extensions::ExtensionRegistry& registry)
      : registry_(registry) {}
  CommandResult submit(const SemanticCommand& command);
  std::optional<AcceptedSemanticCommand> take_next_accepted();

 private:
  friend class AuthoritativeRobotCore;
  struct SessionState {
    std::string package_id;
    std::string logical_device_instance_id;
    std::uint64_t session_id{0};
    std::uint64_t last_sequence{0};
    bool active{false};
  };
  CommandSessionResult bind_session_authoritative(
      const std::string& package_id,
      const std::string& logical_device_instance_id,
      std::uint64_t session_id);
  const extensions::ExtensionRegistry& registry_;
  std::vector<SessionState> sessions_;
  std::vector<AcceptedSemanticCommand> accepted_commands_;
};

enum class EventCategory {
  command_accepted,
  command_rejected,
  capability_changed,
  sensor_observation,
  lifecycle_changed,
};
enum class EventSourceType { authoritative_core, extension, controller };

struct RobotEvent {
  std::uint64_t event_id{0};
  EventCategory category{EventCategory::sensor_observation};
  EventSourceType source_type{EventSourceType::authoritative_core};
  std::string source_id;
  std::string detail;
};

enum class EventResult {
  published,
  rejected_unknown_category,
  rejected_unknown_source,
  rejected_invalid_event,
};

class EventJournal {
 public:
  EventResult publish(const RobotEvent& event);
  const std::vector<RobotEvent>& events() const { return events_; }

 private:
  std::vector<RobotEvent> events_;
};

enum class RobotStateCategory {
  operational,
  motion,
  presentation,
  audio,
  sensors,
};

struct RobotStateSnapshot {
  RobotStateCategory category{RobotStateCategory::operational};
  std::uint64_t generation{0};
  std::string semantic_value;
};

enum class StateResult {
  accepted,
  rejected_unknown_category,
  rejected_invalid_state,
  rejected_stale_generation,
};

class RobotStateStore {
 public:
  const RobotStateSnapshot* current(RobotStateCategory category) const;

 private:
  friend class AuthoritativeRobotCore;
  StateResult update_authoritative(const RobotStateSnapshot& snapshot);
  std::vector<RobotStateSnapshot> snapshots_;
};

}  // namespace zie::api
