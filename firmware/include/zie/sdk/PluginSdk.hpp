#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace zie::sdk {

struct SemanticVersion {
  std::uint16_t major{0};
  std::uint16_t minor{0};
  std::uint16_t patch{0};
};

struct ContractRange {
  SemanticVersion minimum{};
  SemanticVersion maximum{};
};

constexpr SemanticVersion sdk_api_version() { return {1, 0, 0}; }
constexpr ContractRange supported_contract_range() {
  return {{1, 0, 0}, {1, 0, 0}};
}

enum class ContractCompatibility {
  compatible,
  invalid_range,
  incompatible_major,
  unsupported_version,
};

ContractCompatibility check_contract_compatibility(ContractRange requested);

enum class TrustClass {
  built_in,
  signed_trusted,
  local_developer,
  community_untrusted,
};

enum class ExtensionLifecycle {
  declared,
  validated,
  initialized,
  active,
  inactive,
  failed,
  quarantined,
  removed,
};

enum class CallResult {
  accepted,
  no_data,
  rejected_unknown_domain,
  rejected_invalid_request,
  rejected_inactive,
  rejected_unauthorized,
  rejected_stale_context,
  rejected_duplicate,
  rejected_backend,
  unavailable,
};

enum class CommandDomain {
  motion,
  stop_motion,
  expression,
  speech,
  audio_cue,
  sensor_query,
};

struct MotionCommand {
  float normalized_linear{0.0F};
  float normalized_angular{0.0F};
  std::uint32_t lease_ms{0};
};
struct StopMotionCommand {};
struct ExpressionCommand { std::string expression; };
struct SpeechCommand { std::string utterance; };
struct AudioCueCommand { std::string cue; };
struct SensorQueryCommand { std::string semantic_sensor; };
using CommandPayload =
    std::variant<MotionCommand, StopMotionCommand, ExpressionCommand,
                 SpeechCommand, AudioCueCommand, SensorQueryCommand>;
struct SemanticCommand {
  CommandDomain domain{CommandDomain::sensor_query};
  CommandPayload payload{SensorQueryCommand{}};
};

enum class EventDomain {
  command_accepted,
  command_rejected,
  capability_changed,
  sensor_observation,
  lifecycle_changed,
};
struct Event {
  std::uint64_t id{0};
  EventDomain domain{EventDomain::sensor_observation};
  std::string source_id;
  std::string detail;
};
using EventCallback = std::function<void(const Event&)>;

enum class RobotStateDomain {
  operational,
  motion,
  presentation,
  audio,
  sensors,
};
struct RobotState {
  RobotStateDomain domain{RobotStateDomain::operational};
  std::uint64_t generation{0};
  std::string semantic_value;
};

enum class ConfigurationDomain { semantic_text, bounded_integer, boolean };
struct ConfigurationValue {
  std::string key;
  ConfigurationDomain domain{ConfigurationDomain::semantic_text};
  std::string value;
};
struct ConfigurationSnapshot {
  std::uint64_t revision{0};
  std::uint64_t generation{0};
  std::vector<ConfigurationValue> values;
};

enum class ProviderDomain { llm, stt, tts, wake };
struct LlmRequest { std::string prompt; };
struct SttRequest { std::string utterance_id; };
struct TtsRequest { std::string text; };
struct WakeRequest { std::string observation_id; };
using ProviderRequest =
    std::variant<LlmRequest, SttRequest, TtsRequest, WakeRequest>;
struct ProviderInvocation {
  ProviderDomain domain{ProviderDomain::llm};
  std::string semantic_capability;
  ProviderRequest request{LlmRequest{}};
};
struct LlmResponse { std::string text; };
struct SttResponse { std::string transcript; };
struct TtsResponse { std::string utterance_id; };
struct WakeResponse { bool detected{false}; };
using ProviderResponse =
    std::variant<LlmResponse, SttResponse, TtsResponse, WakeResponse>;
struct ProviderOutcome {
  CallResult result{CallResult::rejected_backend};
  ProviderResponse response{LlmResponse{}};
  std::string provider_package_id;
};

struct PresentationState {
  std::uint64_t generation{0};
  std::string semantic_value;
};

class SemanticCommandService {
 public:
  virtual ~SemanticCommandService() = default;
  virtual CallResult submit(const SemanticCommand& command) = 0;
};

class EventService {
 public:
  virtual ~EventService() = default;
  virtual CallResult subscribe(const std::string& local_subscription_id,
                               const std::vector<EventDomain>& domains) = 0;
  virtual CallResult deliver_next(const std::string& local_subscription_id,
                                  const EventCallback& callback) = 0;
  virtual CallResult unsubscribe(
      const std::string& local_subscription_id) = 0;
};

class RobotStateService {
 public:
  virtual ~RobotStateService() = default;
  virtual std::optional<RobotState> current(RobotStateDomain domain) = 0;
};

class ConfigurationService {
 public:
  virtual ~ConfigurationService() = default;
  virtual std::optional<ConfigurationSnapshot> active() = 0;
  virtual CallResult stage(std::uint64_t revision,
                           const std::vector<ConfigurationValue>& values) = 0;
};

class ProviderService {
 public:
  virtual ~ProviderService() = default;
  virtual ProviderOutcome invoke(const ProviderInvocation& invocation) = 0;
};

class PresentationService {
 public:
  virtual ~PresentationService() = default;
  virtual std::optional<PresentationState> current() = 0;
};

class ExtensionContext {
 public:
  virtual ~ExtensionContext() = default;
  virtual const std::string& package_id() const = 0;
  virtual const std::string& logical_device_id() const = 0;
  virtual const std::string& hardware_profile_id() const = 0;
  virtual TrustClass trust() const = 0;
  virtual ExtensionLifecycle lifecycle() const = 0;
  virtual SemanticVersion contract_version() const = 0;
  virtual std::uint64_t instance_epoch() const = 0;
  virtual const std::vector<std::string>& validated_capabilities() const = 0;
  virtual const std::vector<std::string>& active_capabilities() const = 0;
  virtual std::shared_ptr<SemanticCommandService> commands() const = 0;
  virtual std::shared_ptr<EventService> events() const = 0;
  virtual std::shared_ptr<RobotStateService> robot_state() const = 0;
  virtual std::shared_ptr<ConfigurationService> configuration() const = 0;
  virtual std::shared_ptr<ProviderService> providers() const = 0;
  virtual std::shared_ptr<PresentationService> presentation() const = 0;
};

class Extension {
 public:
  virtual ~Extension() = default;
  virtual bool initialize(std::shared_ptr<const ExtensionContext> context) = 0;
  virtual bool activate(std::shared_ptr<const ExtensionContext> context) = 0;
  virtual void suspended() = 0;
};

}  // namespace zie::sdk
