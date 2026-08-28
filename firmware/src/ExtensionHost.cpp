#include "core/ExtensionHost.hpp"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/AuthoritativeRobotCore.hpp"
#include "zie/core/HardwareProfile.hpp"

namespace zie::core {
namespace {

using extensions::ExtensionCategory;
using extensions::LifecycleState;

bool contains(const std::vector<std::string>& values,
              const std::string& value) {
  return std::find(values.begin(), values.end(), value) != values.end();
}

bool begins_with(const std::string& value, const std::string& prefix) {
  return value.compare(0, prefix.size(), prefix) == 0;
}

bool provider_publication_capability(const std::string& capability) {
  return begins_with(capability, "provider.llm.") ||
         begins_with(capability, "provider.stt.") ||
         begins_with(capability, "provider.tts.") ||
         begins_with(capability, "provider.wake.");
}

bool known_capability(const std::string& capability) {
  static const std::vector<std::string> known{
      "semantic.motion",       "semantic.presentation",
      "semantic.audio",        "semantic.sensor-query",
      "events.read",           "robot.state.read",
      "configuration.read-stage", "provider.invoke.llm",
      "provider.invoke.stt",   "provider.invoke.tts",
      "provider.invoke.wake",  "presentation.read"};
  return contains(known, capability) ||
         provider_publication_capability(capability);
}

bool general_host_category(const ExtensionCategory category) {
  switch (category) {
    case ExtensionCategory::behavior:
    case ExtensionCategory::ai_tool:
    case ExtensionCategory::automation:
    case ExtensionCategory::integration:
      return true;
    case ExtensionCategory::hardware_display:
    case ExtensionCategory::hardware_camera:
    case ExtensionCategory::hardware_audio_input:
    case ExtensionCategory::hardware_audio_output:
    case ExtensionCategory::hardware_range:
    case ExtensionCategory::hardware_proximity:
    case ExtensionCategory::hardware_cliff:
    case ExtensionCategory::hardware_imu:
    case ExtensionCategory::hardware_drive:
    case ExtensionCategory::hardware_motor_driver:
    case ExtensionCategory::hardware_actuator:
    case ExtensionCategory::hardware_arm:
    case ExtensionCategory::hardware_gripper:
    case ExtensionCategory::hardware_tool:
    case ExtensionCategory::hardware_power:
    case ExtensionCategory::hardware_battery:
    case ExtensionCategory::hardware_rgb:
    case ExtensionCategory::hardware_touch:
    case ExtensionCategory::hardware_network:
    case ExtensionCategory::ai_provider:
    case ExtensionCategory::voice_stt:
    case ExtensionCategory::voice_tts:
    case ExtensionCategory::voice_wakeword:
    case ExtensionCategory::face_pack:
    case ExtensionCategory::emotion_pack:
    case ExtensionCategory::rgb_pack:
    case ExtensionCategory::sound_pack:
      return false;
  }
  return false;
}

bool category_allows(const ExtensionCategory category,
                     const std::string& capability) {
  if (!known_capability(capability)) return false;
  if (capability == "semantic.motion") {
    return category == ExtensionCategory::behavior ||
           category == ExtensionCategory::automation ||
           category == ExtensionCategory::integration;
  }
  if (provider_publication_capability(capability)) {
    if (begins_with(capability, "provider.wake.")) {
      return category == ExtensionCategory::voice_wakeword;
    }
    if (begins_with(capability, "provider.stt.")) {
      return category == ExtensionCategory::voice_stt;
    }
    if (begins_with(capability, "provider.tts.")) {
      return category == ExtensionCategory::voice_tts;
    }
    return category == ExtensionCategory::ai_provider;
  }
  if (begins_with(capability, "provider.invoke.")) {
    return general_host_category(category);
  }
  if (category == ExtensionCategory::voice_wakeword ||
      category == ExtensionCategory::voice_stt ||
      category == ExtensionCategory::voice_tts ||
      category == ExtensionCategory::ai_provider) {
    return capability == "events.read" ||
           capability == "robot.state.read" ||
           capability == "configuration.read-stage";
  }
  return general_host_category(category);
}

bool known_failure(const extensions::FailureClass failure) {
  switch (failure) {
    case extensions::FailureClass::none:
    case extensions::FailureClass::temporary:
    case extensions::FailureClass::configuration:
    case extensions::FailureClass::authentication:
    case extensions::FailureClass::incompatible:
    case extensions::FailureClass::hardware_fault:
    case extensions::FailureClass::security:
      return true;
  }
  return false;
}

sdk::TrustClass sdk_trust(const extensions::TrustClass trust) {
  switch (trust) {
    case extensions::TrustClass::built_in:
      return sdk::TrustClass::built_in;
    case extensions::TrustClass::signed_trusted:
      return sdk::TrustClass::signed_trusted;
    case extensions::TrustClass::local_developer:
      return sdk::TrustClass::local_developer;
    case extensions::TrustClass::community_untrusted:
      return sdk::TrustClass::community_untrusted;
  }
  return sdk::TrustClass::community_untrusted;
}

struct SessionAuthority {
  extensions::ExtensionRegistry* registry{nullptr};
  api::ResilientEventBus* event_bus{nullptr};
  std::string package_id;
  std::string logical_device_id;
  std::uint64_t epoch{0};
  std::uint64_t registry_generation{0};
  bool current{false};
  std::vector<std::string> event_subscriptions;
};

class IssuedExtensionContext final : public sdk::ExtensionContext {
 public:
  IssuedExtensionContext(
      std::string package_id, std::string logical_device_id,
      std::string hardware_profile_id, const sdk::TrustClass trust,
      const sdk::ExtensionLifecycle lifecycle, const std::uint64_t epoch,
      std::vector<std::string> validated, std::vector<std::string> active,
      std::shared_ptr<sdk::SemanticCommandService> commands,
      std::shared_ptr<sdk::EventService> events,
      std::shared_ptr<sdk::RobotStateService> state,
      std::shared_ptr<sdk::ConfigurationService> configuration,
      std::shared_ptr<sdk::ProviderService> providers,
      std::shared_ptr<sdk::PresentationService> presentation)
      : package_id_(std::move(package_id)),
        logical_device_id_(std::move(logical_device_id)),
        hardware_profile_id_(std::move(hardware_profile_id)),
        trust_(trust), lifecycle_(lifecycle), epoch_(epoch),
        validated_(std::move(validated)), active_(std::move(active)),
        commands_(std::move(commands)), events_(std::move(events)),
        state_(std::move(state)), configuration_(std::move(configuration)),
        providers_(std::move(providers)),
        presentation_(std::move(presentation)) {}
  const std::string& package_id() const override { return package_id_; }
  const std::string& logical_device_id() const override {
    return logical_device_id_;
  }
  const std::string& hardware_profile_id() const override {
    return hardware_profile_id_;
  }
  sdk::TrustClass trust() const override { return trust_; }
  sdk::ExtensionLifecycle lifecycle() const override { return lifecycle_; }
  sdk::SemanticVersion contract_version() const override {
    return sdk::sdk_api_version();
  }
  std::uint64_t instance_epoch() const override { return epoch_; }
  const std::vector<std::string>& validated_capabilities() const override {
    return validated_;
  }
  const std::vector<std::string>& active_capabilities() const override {
    return active_;
  }
  std::shared_ptr<sdk::SemanticCommandService> commands() const override {
    return commands_;
  }
  std::shared_ptr<sdk::EventService> events() const override {
    return events_;
  }
  std::shared_ptr<sdk::RobotStateService> robot_state() const override {
    return state_;
  }
  std::shared_ptr<sdk::ConfigurationService> configuration() const override {
    return configuration_;
  }
  std::shared_ptr<sdk::ProviderService> providers() const override {
    return providers_;
  }
  std::shared_ptr<sdk::PresentationService> presentation() const override {
    return presentation_;
  }

 private:
  std::string package_id_;
  std::string logical_device_id_;
  std::string hardware_profile_id_;
  sdk::TrustClass trust_;
  sdk::ExtensionLifecycle lifecycle_;
  std::uint64_t epoch_;
  std::vector<std::string> validated_;
  std::vector<std::string> active_;
  std::shared_ptr<sdk::SemanticCommandService> commands_;
  std::shared_ptr<sdk::EventService> events_;
  std::shared_ptr<sdk::RobotStateService> state_;
  std::shared_ptr<sdk::ConfigurationService> configuration_;
  std::shared_ptr<sdk::ProviderService> providers_;
  std::shared_ptr<sdk::PresentationService> presentation_;
};

sdk::CallResult authorize(const std::shared_ptr<SessionAuthority>& authority,
                          const std::string& capability) {
  if (!authority->current) return sdk::CallResult::rejected_stale_context;
  const auto* record = authority->registry->find(authority->package_id);
  if (record == nullptr || record->device_identity.logical.instance_id !=
                               authority->logical_device_id) {
    return sdk::CallResult::rejected_stale_context;
  }
  if (record->authorization_generation != authority->registry_generation) {
    return sdk::CallResult::rejected_stale_context;
  }
  if (record->lifecycle != LifecycleState::active) {
    return sdk::CallResult::rejected_inactive;
  }
  if (!contains(record->active_capabilities, capability)) {
    return sdk::CallResult::rejected_unauthorized;
  }
  return sdk::CallResult::accepted;
}

const char* command_capability(const sdk::CommandDomain domain) {
  switch (domain) {
    case sdk::CommandDomain::motion:
    case sdk::CommandDomain::stop_motion:
      return "semantic.motion";
    case sdk::CommandDomain::expression:
      return "semantic.presentation";
    case sdk::CommandDomain::speech:
    case sdk::CommandDomain::audio_cue:
      return "semantic.audio";
    case sdk::CommandDomain::sensor_query:
      return "semantic.sensor-query";
  }
  return nullptr;
}

bool make_command(const sdk::SemanticCommand& source,
                  const SessionAuthority& authority,
                  const std::uint64_t sequence,
                  api::SemanticCommand& target) {
  target.source = {authority.package_id, authority.logical_device_id,
                   authority.epoch, sequence};
  switch (source.domain) {
    case sdk::CommandDomain::motion: {
      const auto* value = std::get_if<sdk::MotionCommand>(&source.payload);
      if (value == nullptr) return false;
      target.type = api::SemanticCommandType::motion_intent;
      target.payload = api::MotionIntent{value->normalized_linear,
                                         value->normalized_angular,
                                         value->lease_ms};
      return true;
    }
    case sdk::CommandDomain::stop_motion:
      if (!std::holds_alternative<sdk::StopMotionCommand>(source.payload)) {
        return false;
      }
      target.type = api::SemanticCommandType::stop_motion;
      target.payload = api::StopMotionIntent{};
      return true;
    case sdk::CommandDomain::expression: {
      const auto* value = std::get_if<sdk::ExpressionCommand>(&source.payload);
      if (value == nullptr) return false;
      target.type = api::SemanticCommandType::expression_intent;
      target.payload = api::ExpressionIntent{value->expression};
      return true;
    }
    case sdk::CommandDomain::speech: {
      const auto* value = std::get_if<sdk::SpeechCommand>(&source.payload);
      if (value == nullptr) return false;
      target.type = api::SemanticCommandType::audio_speech_intent;
      target.payload = api::AudioSpeechIntent{value->utterance};
      return true;
    }
    case sdk::CommandDomain::audio_cue: {
      const auto* value = std::get_if<sdk::AudioCueCommand>(&source.payload);
      if (value == nullptr) return false;
      target.type = api::SemanticCommandType::audio_cue_intent;
      target.payload = api::AudioCueIntent{value->cue};
      return true;
    }
    case sdk::CommandDomain::sensor_query: {
      const auto* value = std::get_if<sdk::SensorQueryCommand>(&source.payload);
      if (value == nullptr) return false;
      target.type = api::SemanticCommandType::sensor_query;
      target.payload = api::SensorQueryIntent{value->semantic_sensor};
      return true;
    }
  }
  return false;
}

class CommandAdapter final : public sdk::SemanticCommandService {
 public:
  CommandAdapter(std::shared_ptr<SessionAuthority> authority,
                 api::SemanticRobotApi& api)
      : authority_(std::move(authority)), api_(api) {}
  sdk::CallResult submit(const sdk::SemanticCommand& command) override {
    const char* const capability = command_capability(command.domain);
    if (capability == nullptr) {
      return sdk::CallResult::rejected_unknown_domain;
    }
    const auto allowed = authorize(authority_, capability);
    if (allowed != sdk::CallResult::accepted) return allowed;
    api::SemanticCommand translated;
    if (!make_command(command, *authority_, sequence_ + 1, translated)) {
      return sdk::CallResult::rejected_invalid_request;
    }
    const auto result = api_.submit(translated);
    if (result == api::CommandResult::accepted) {
      ++sequence_;
      return sdk::CallResult::accepted;
    }
    return result == api::CommandResult::rejected_inactive_issuer
               ? sdk::CallResult::rejected_inactive
               : sdk::CallResult::rejected_backend;
  }

 private:
  std::shared_ptr<SessionAuthority> authority_;
  api::SemanticRobotApi& api_;
  std::uint64_t sequence_{0};
};

bool event_category(const sdk::EventDomain source, api::EventCategory& target) {
  switch (source) {
    case sdk::EventDomain::command_accepted:
      target = api::EventCategory::command_accepted;
      return true;
    case sdk::EventDomain::command_rejected:
      target = api::EventCategory::command_rejected;
      return true;
    case sdk::EventDomain::capability_changed:
      target = api::EventCategory::capability_changed;
      return true;
    case sdk::EventDomain::sensor_observation:
      target = api::EventCategory::sensor_observation;
      return true;
    case sdk::EventDomain::lifecycle_changed:
      target = api::EventCategory::lifecycle_changed;
      return true;
  }
  return false;
}

sdk::EventDomain sdk_event_category(const api::EventCategory source) {
  switch (source) {
    case api::EventCategory::command_accepted:
      return sdk::EventDomain::command_accepted;
    case api::EventCategory::command_rejected:
      return sdk::EventDomain::command_rejected;
    case api::EventCategory::capability_changed:
      return sdk::EventDomain::capability_changed;
    case api::EventCategory::sensor_observation:
      return sdk::EventDomain::sensor_observation;
    case api::EventCategory::lifecycle_changed:
      return sdk::EventDomain::lifecycle_changed;
  }
  return static_cast<sdk::EventDomain>(-1);
}

class EventAdapter final : public sdk::EventService {
 public:
  EventAdapter(std::shared_ptr<SessionAuthority> authority,
               api::ResilientEventBus& bus)
      : authority_(std::move(authority)), bus_(bus) {}
  sdk::CallResult subscribe(
      const std::string& local_id,
      const std::vector<sdk::EventDomain>& domains) override {
    const auto allowed = authorize(authority_, "events.read");
    if (allowed != sdk::CallResult::accepted) return allowed;
    std::vector<api::EventCategory> translated;
    for (const auto domain : domains) {
      api::EventCategory category{};
      if (!event_category(domain, category)) {
        return sdk::CallResult::rejected_unknown_domain;
      }
      translated.push_back(category);
    }
    const auto result = bus_.subscribe(
        {{global_id(local_id), authority_->package_id,
          authority_->logical_device_id},
         "events.read", translated});
    if (result == api::EventBusResult::subscribed) {
      authority_->event_subscriptions.push_back(global_id(local_id));
      return sdk::CallResult::accepted;
    }
    return result == api::EventBusResult::rejected_duplicate_subscriber
               ? sdk::CallResult::rejected_duplicate
               : sdk::CallResult::rejected_backend;
  }
  sdk::CallResult deliver_next(const std::string& local_id,
                               const sdk::EventCallback& callback) override {
    const auto allowed = authorize(authority_, "events.read");
    if (allowed != sdk::CallResult::accepted) return allowed;
    const auto result = bus_.deliver_next(
        global_id(local_id), [&callback](const api::RobotEvent& event) {
          callback({event.event_id, sdk_event_category(event.category),
                    event.source_id, event.detail});
        });
    if (result == api::EventBusResult::delivered) {
      return sdk::CallResult::accepted;
    }
    return result == api::EventBusResult::rejected_no_event
               ? sdk::CallResult::no_data
               : sdk::CallResult::rejected_backend;
  }
  sdk::CallResult unsubscribe(const std::string& local_id) override {
    const auto allowed = authorize(authority_, "events.read");
    if (allowed != sdk::CallResult::accepted) return allowed;
    const auto id = global_id(local_id);
    if (bus_.unsubscribe(id) != api::EventBusResult::unsubscribed) {
      return sdk::CallResult::rejected_backend;
    }
    authority_->event_subscriptions.erase(
        std::remove(authority_->event_subscriptions.begin(),
                    authority_->event_subscriptions.end(), id),
        authority_->event_subscriptions.end());
    return sdk::CallResult::accepted;
  }

 private:
  std::string global_id(const std::string& local_id) const {
    return authority_->package_id + "." + std::to_string(authority_->epoch) +
           "." + local_id;
  }
  std::shared_ptr<SessionAuthority> authority_;
  api::ResilientEventBus& bus_;
};

bool state_category(const sdk::RobotStateDomain source,
                    api::RobotStateCategory& target) {
  switch (source) {
    case sdk::RobotStateDomain::operational:
      target = api::RobotStateCategory::operational;
      return true;
    case sdk::RobotStateDomain::motion:
      target = api::RobotStateCategory::motion;
      return true;
    case sdk::RobotStateDomain::presentation:
      target = api::RobotStateCategory::presentation;
      return true;
    case sdk::RobotStateDomain::audio:
      target = api::RobotStateCategory::audio;
      return true;
    case sdk::RobotStateDomain::sensors:
      target = api::RobotStateCategory::sensors;
      return true;
  }
  return false;
}

class StateAdapter final : public sdk::RobotStateService {
 public:
  StateAdapter(std::shared_ptr<SessionAuthority> authority,
               api::RobotStateStore& state)
      : authority_(std::move(authority)), state_(state) {}
  std::optional<sdk::RobotState> current(
      const sdk::RobotStateDomain domain) override {
    if (authorize(authority_, "robot.state.read") !=
        sdk::CallResult::accepted) {
      return std::nullopt;
    }
    api::RobotStateCategory translated{};
    if (!state_category(domain, translated)) return std::nullopt;
    const auto* snapshot = state_.current(translated);
    if (snapshot == nullptr) return std::nullopt;
    return sdk::RobotState{domain, snapshot->generation,
                           snapshot->semantic_value};
  }

 private:
  std::shared_ptr<SessionAuthority> authority_;
  api::RobotStateStore& state_;
};

bool configuration_domain(const sdk::ConfigurationDomain source,
                          extensions::ConfigurationValueDomain& target) {
  switch (source) {
    case sdk::ConfigurationDomain::semantic_text:
      target = extensions::ConfigurationValueDomain::semantic_text;
      return true;
    case sdk::ConfigurationDomain::bounded_integer:
      target = extensions::ConfigurationValueDomain::bounded_integer;
      return true;
    case sdk::ConfigurationDomain::boolean:
      target = extensions::ConfigurationValueDomain::boolean;
      return true;
  }
  return false;
}

sdk::ConfigurationDomain sdk_configuration_domain(
    const extensions::ConfigurationValueDomain source) {
  switch (source) {
    case extensions::ConfigurationValueDomain::semantic_text:
      return sdk::ConfigurationDomain::semantic_text;
    case extensions::ConfigurationValueDomain::bounded_integer:
      return sdk::ConfigurationDomain::bounded_integer;
    case extensions::ConfigurationValueDomain::boolean:
      return sdk::ConfigurationDomain::boolean;
  }
  return static_cast<sdk::ConfigurationDomain>(-1);
}

class ConfigurationAdapter final : public sdk::ConfigurationService {
 public:
  ConfigurationAdapter(std::shared_ptr<SessionAuthority> authority,
                       extensions::TransactionalConfiguration& configuration,
                       std::string profile)
      : authority_(std::move(authority)), configuration_(configuration),
        profile_(std::move(profile)) {}
  std::optional<sdk::ConfigurationSnapshot> active() override {
    if (authorize(authority_, "configuration.read-stage") !=
        sdk::CallResult::accepted) {
      return std::nullopt;
    }
    const auto* record = configuration_.find(authority_->package_id);
    if (record == nullptr || !record->active_configuration.has_value()) {
      return std::nullopt;
    }
    if (record->authoritative_binding.package_id != authority_->package_id ||
        record->authoritative_binding.logical_device_instance_id !=
            authority_->logical_device_id ||
        record->authoritative_binding.hardware_profile_id != profile_) {
      return std::nullopt;
    }
    sdk::ConfigurationSnapshot result;
    result.revision = record->active_configuration->revision;
    result.generation = record->active_configuration->generation;
    for (const auto& value : record->active_configuration->values) {
      result.values.push_back({value.key, sdk_configuration_domain(value.domain),
                               value.value});
    }
    return result;
  }
  sdk::CallResult stage(
      const std::uint64_t revision,
      const std::vector<sdk::ConfigurationValue>& values) override {
    const auto allowed = authorize(authority_, "configuration.read-stage");
    if (allowed != sdk::CallResult::accepted) return allowed;
    std::vector<extensions::ConfigurationValue> translated;
    for (const auto& value : values) {
      extensions::ConfigurationValueDomain domain{};
      if (!configuration_domain(value.domain, domain)) {
        return sdk::CallResult::rejected_unknown_domain;
      }
      translated.push_back({value.key, domain, value.value});
    }
    const extensions::ConfigurationBinding binding{
        authority_->package_id, authority_->logical_device_id, profile_};
    const auto result = configuration_.stage(
        {binding, revision, translated}, {binding});
    return result == extensions::ConfigurationResult::staged
               ? sdk::CallResult::accepted
               : sdk::CallResult::rejected_backend;
  }

 private:
  std::shared_ptr<SessionAuthority> authority_;
  extensions::TransactionalConfiguration& configuration_;
  std::string profile_;
};

const char* provider_invoke_capability(const sdk::ProviderDomain domain) {
  switch (domain) {
    case sdk::ProviderDomain::llm:
      return "provider.invoke.llm";
    case sdk::ProviderDomain::stt:
      return "provider.invoke.stt";
    case sdk::ProviderDomain::tts:
      return "provider.invoke.tts";
    case sdk::ProviderDomain::wake:
      return "provider.invoke.wake";
  }
  return nullptr;
}

bool provider_request(const sdk::ProviderInvocation& source,
                      providers::ProviderInvocation& target) {
  target.requested_capability = source.semantic_capability;
  switch (source.domain) {
    case sdk::ProviderDomain::llm: {
      const auto* value = std::get_if<sdk::LlmRequest>(&source.request);
      if (value == nullptr) return false;
      target.request = providers::LlmRequest{value->prompt};
      return true;
    }
    case sdk::ProviderDomain::stt: {
      const auto* value = std::get_if<sdk::SttRequest>(&source.request);
      if (value == nullptr) return false;
      target.request = providers::SttRequest{value->utterance_id};
      return true;
    }
    case sdk::ProviderDomain::tts: {
      const auto* value = std::get_if<sdk::TtsRequest>(&source.request);
      if (value == nullptr) return false;
      target.request = providers::TtsRequest{value->text};
      return true;
    }
    case sdk::ProviderDomain::wake: {
      const auto* value = std::get_if<sdk::WakeRequest>(&source.request);
      if (value == nullptr) return false;
      target.request = providers::WakeRequest{value->observation_id};
      return true;
    }
  }
  return false;
}

sdk::ProviderResponse provider_response(
    const providers::ProviderResponse& response) {
  if (const auto* value = std::get_if<providers::LlmResponse>(&response)) {
    return sdk::LlmResponse{value->text};
  }
  if (const auto* value = std::get_if<providers::SttResponse>(&response)) {
    return sdk::SttResponse{value->transcript};
  }
  if (const auto* value = std::get_if<providers::TtsResponse>(&response)) {
    return sdk::TtsResponse{value->utterance_id};
  }
  return sdk::WakeResponse{std::get<providers::WakeResponse>(response).detected};
}

class ProviderAdapter final : public sdk::ProviderService {
 public:
  ProviderAdapter(std::shared_ptr<SessionAuthority> authority,
                  providers::ProviderRouter& router)
      : authority_(std::move(authority)), router_(router) {}
  sdk::ProviderOutcome invoke(
      const sdk::ProviderInvocation& invocation) override {
    const char* const capability = provider_invoke_capability(invocation.domain);
    if (capability == nullptr) {
      return {sdk::CallResult::rejected_unknown_domain, sdk::LlmResponse{}, {}};
    }
    const auto allowed = authorize(authority_, capability);
    if (allowed != sdk::CallResult::accepted) {
      return {allowed, sdk::LlmResponse{}, {}};
    }
    providers::ProviderInvocation translated;
    if (!provider_request(invocation, translated)) {
      return {sdk::CallResult::rejected_invalid_request, sdk::LlmResponse{},
              {}};
    }
    const auto outcome = router_.invoke(translated);
    return {outcome.result == providers::ProviderResult::succeeded
                ? sdk::CallResult::accepted
                : sdk::CallResult::rejected_backend,
            provider_response(outcome.response), outcome.provider_package_id};
  }

 private:
  std::shared_ptr<SessionAuthority> authority_;
  providers::ProviderRouter& router_;
};

class PresentationAdapter final : public sdk::PresentationService {
 public:
  PresentationAdapter(std::shared_ptr<SessionAuthority> authority,
                      api::RobotStateStore& state)
      : authority_(std::move(authority)), state_(state) {}
  std::optional<sdk::PresentationState> current() override {
    if (authorize(authority_, "presentation.read") !=
        sdk::CallResult::accepted) {
      return std::nullopt;
    }
    const auto* snapshot =
        state_.current(api::RobotStateCategory::presentation);
    if (snapshot == nullptr) return std::nullopt;
    return sdk::PresentationState{snapshot->generation,
                                  snapshot->semantic_value};
  }

 private:
  std::shared_ptr<SessionAuthority> authority_;
  api::RobotStateStore& state_;
};

bool has_command_capability(const std::vector<std::string>& capabilities) {
  return contains(capabilities, "semantic.motion") ||
         contains(capabilities, "semantic.presentation") ||
         contains(capabilities, "semantic.audio") ||
         contains(capabilities, "semantic.sensor-query");
}

void notify_suspended(const std::shared_ptr<sdk::Extension>& extension) {
  try {
    extension->suspended();
  } catch (...) {
    // Lifecycle revocation is already authoritative; plugin cleanup cannot undo it.
  }
}

}  // namespace

class ExtensionHost::Impl {
 public:
  struct Record {
    std::string package_id;
    std::shared_ptr<sdk::Extension> extension;
    sdk::ExtensionLifecycle lifecycle{sdk::ExtensionLifecycle::declared};
    std::shared_ptr<SessionAuthority> authority;
    std::shared_ptr<const sdk::ExtensionContext> context;
  };

  Impl(extensions::ExtensionRegistry& registry,
       api::AuthoritativeRobotCore& core, api::SemanticRobotApi& commands,
       api::ResilientEventBus& events, api::RobotStateStore& robot_state,
       extensions::TransactionalConfiguration& configuration,
       providers::ProviderRouter& providers,
       HardwareProfileManager* hardware_profiles)
      : registry(registry), core(core), commands(commands), events(events),
        robot_state(robot_state), configuration(configuration),
        providers(providers), hardware_profiles(hardware_profiles) {}

  Record* find(const std::string& package_id) {
    const auto found = std::find_if(records.rbegin(), records.rend(),
                                    [&package_id](const Record& record) {
                                      return record.package_id == package_id &&
                                             record.lifecycle !=
                                                 sdk::ExtensionLifecycle::removed;
                                    });
    return found == records.rend() ? nullptr : &*found;
  }
  const Record* find(const std::string& package_id) const {
    const auto found = std::find_if(records.rbegin(), records.rend(),
                                    [&package_id](const Record& record) {
                                      return record.package_id == package_id &&
                                             record.lifecycle !=
                                                 sdk::ExtensionLifecycle::removed;
                                    });
    return found == records.rend() ? nullptr : &*found;
  }
  void revoke(Record& record) {
    if (record.authority != nullptr) {
      record.authority->current = false;
      if (record.authority->event_bus != nullptr) {
        for (const auto& id : record.authority->event_subscriptions) {
          record.authority->event_bus->unsubscribe(id);
        }
      }
      record.authority->event_subscriptions.clear();
    }
    record.authority.reset();
  }
  void refresh_inactive_context(Record& record,
                                const sdk::ExtensionLifecycle lifecycle) {
    const auto* registry_record = registry.find(record.package_id);
    if (registry_record == nullptr ||
        registry_record->lifecycle == LifecycleState::removed) {
      record.context.reset();
      return;
    }
    record.context = make_context(*registry_record, lifecycle, {}, nullptr);
  }
  std::shared_ptr<const sdk::ExtensionContext> make_context(
      const extensions::ExtensionRecord& registry_record,
      const sdk::ExtensionLifecycle lifecycle,
      const std::vector<std::string>& active,
      const std::shared_ptr<SessionAuthority>& authority) {
    std::shared_ptr<sdk::SemanticCommandService> command_service;
    std::shared_ptr<sdk::EventService> event_service;
    std::shared_ptr<sdk::RobotStateService> state_service;
    std::shared_ptr<sdk::ConfigurationService> configuration_service;
    std::shared_ptr<sdk::ProviderService> provider_service;
    std::shared_ptr<sdk::PresentationService> presentation_service;
    if (authority != nullptr && has_command_capability(active)) {
      command_service = std::make_shared<CommandAdapter>(authority, commands);
    }
    if (authority != nullptr && contains(active, "events.read")) {
      event_service = std::make_shared<EventAdapter>(authority, events);
    }
    if (authority != nullptr && contains(active, "robot.state.read")) {
      state_service = std::make_shared<StateAdapter>(authority, robot_state);
    }
    if (authority != nullptr &&
        contains(active, "configuration.read-stage")) {
      configuration_service = std::make_shared<ConfigurationAdapter>(
          authority, configuration,
          registry_record.device_identity.hardware_profile.profile_id);
    }
    if (authority != nullptr &&
        (contains(active, "provider.invoke.llm") ||
         contains(active, "provider.invoke.stt") ||
         contains(active, "provider.invoke.tts") ||
         contains(active, "provider.invoke.wake"))) {
      provider_service =
          std::make_shared<ProviderAdapter>(authority, providers);
    }
    if (authority != nullptr && contains(active, "presentation.read")) {
      presentation_service =
          std::make_shared<PresentationAdapter>(authority, robot_state);
    }
    return std::make_shared<IssuedExtensionContext>(
        registry_record.manifest.id,
        registry_record.device_identity.logical.instance_id,
        registry_record.device_identity.hardware_profile.profile_id,
        sdk_trust(registry_record.assigned_trust), lifecycle,
        authority == nullptr ? registry_record.authorization_generation
                             : authority->epoch,
        registry_record.validated_capabilities, active,
        std::move(command_service), std::move(event_service),
        std::move(state_service), std::move(configuration_service),
        std::move(provider_service), std::move(presentation_service));
  }

  extensions::ExtensionRegistry& registry;
  api::AuthoritativeRobotCore& core;
  api::SemanticRobotApi& commands;
  api::ResilientEventBus& events;
  api::RobotStateStore& robot_state;
  extensions::TransactionalConfiguration& configuration;
  providers::ProviderRouter& providers;
  HardwareProfileManager* hardware_profiles;
  std::vector<Record> records;
};

ExtensionHost::ExtensionHost(
    extensions::ExtensionRegistry& registry, api::AuthoritativeRobotCore& core,
    api::SemanticRobotApi& commands, api::ResilientEventBus& events,
    api::RobotStateStore& robot_state,
    extensions::TransactionalConfiguration& configuration,
    providers::ProviderRouter& providers,
    HardwareProfileManager* hardware_profiles)
    : impl_(std::make_unique<Impl>(registry, core, commands, events, robot_state,
                                   configuration, providers,
                                   hardware_profiles)) {}

ExtensionHost::~ExtensionHost() {
  for (auto& record : impl_->records) {
    impl_->revoke(record);
  }
}

ExtensionHostResult ExtensionHost::declare_extension(
    const extensions::ExtensionCandidate& candidate,
    const extensions::RegistryAssignment& assignment,
    const sdk::ContractRange contract,
    const std::shared_ptr<sdk::Extension>& extension) {
  if (sdk::check_contract_compatibility(contract) !=
      sdk::ContractCompatibility::compatible) {
    return ExtensionHostResult::rejected_contract;
  }
  if (candidate.manifest.extension_class !=
          extensions::ExtensionClass::host_plugin ||
      extension == nullptr) {
    return ExtensionHostResult::rejected_non_executable;
  }
  if (std::any_of(impl_->records.begin(), impl_->records.end(),
                  [&extension](const Impl::Record& record) {
                    return record.extension.get() == extension.get();
                  })) {
    return ExtensionHostResult::rejected_duplicate_instance;
  }
  if (impl_->registry.register_extension(candidate, assignment) !=
      extensions::RegistryResult::accepted) {
    return ExtensionHostResult::rejected_registry;
  }
  impl_->records.push_back(
      {assignment.package_id, extension, sdk::ExtensionLifecycle::declared,
       nullptr, nullptr});
  return ExtensionHostResult::declared;
}

ExtensionHostResult ExtensionHost::validate_extension(
    const std::string& package_id,
    const std::vector<std::string>& validated_capabilities) {
  auto* record = impl_->find(package_id);
  const auto* registry_record = impl_->registry.find(package_id);
  if (record == nullptr || registry_record == nullptr) {
    return ExtensionHostResult::rejected_not_found;
  }
  if (record->lifecycle != sdk::ExtensionLifecycle::declared) {
    return ExtensionHostResult::rejected_lifecycle;
  }
  for (const auto& capability : validated_capabilities) {
    if (!category_allows(registry_record->manifest.category, capability)) {
      return ExtensionHostResult::rejected_capability;
    }
  }
  if (!extensions::validate_capability_state(
           registry_record->manifest,
           {LifecycleState::inactive, extensions::FailureClass::none,
            validated_capabilities, {}})
           .valid) {
    return ExtensionHostResult::rejected_capability;
  }
  if (impl_->registry.transition(package_id, LifecycleState::validated) !=
          extensions::RegistryResult::transitioned ||
      impl_->registry.validate_capabilities(package_id,
                                            validated_capabilities) !=
          extensions::RegistryResult::capabilities_validated) {
    return ExtensionHostResult::rejected_registry;
  }
  record->lifecycle = sdk::ExtensionLifecycle::validated;
  return ExtensionHostResult::validated;
}

ExtensionHostResult ExtensionHost::initialize_extension(
    const std::string& package_id) {
  auto* record = impl_->find(package_id);
  if (record == nullptr) return ExtensionHostResult::rejected_not_found;
  if (record->lifecycle != sdk::ExtensionLifecycle::validated) {
    return ExtensionHostResult::rejected_lifecycle;
  }
  if (impl_->registry.transition(package_id, LifecycleState::inactive) !=
      extensions::RegistryResult::transitioned) {
    return ExtensionHostResult::rejected_registry;
  }
  const auto* registry_record = impl_->registry.find(package_id);
  record->context = impl_->make_context(
      *registry_record, sdk::ExtensionLifecycle::initialized, {}, nullptr);
  bool initialized = false;
  try {
    initialized = record->extension->initialize(record->context);
  } catch (...) {
    initialized = false;
  }
  if (!initialized) {
    impl_->registry.transition(package_id, LifecycleState::activating);
    impl_->registry.transition(package_id, LifecycleState::failed,
                               extensions::FailureClass::configuration);
    record->lifecycle = sdk::ExtensionLifecycle::failed;
    impl_->refresh_inactive_context(*record, sdk::ExtensionLifecycle::failed);
    return ExtensionHostResult::rejected_extension;
  }
  record->lifecycle = sdk::ExtensionLifecycle::initialized;
  return ExtensionHostResult::initialized;
}

ExtensionHostResult ExtensionHost::activate_extension(
    const std::string& package_id,
    const std::vector<std::string>& active_capabilities) {
  auto* record = impl_->find(package_id);
  const auto* registry_record = impl_->registry.find(package_id);
  if (record == nullptr || registry_record == nullptr) {
    return ExtensionHostResult::rejected_not_found;
  }
  if (record->lifecycle != sdk::ExtensionLifecycle::initialized &&
      record->lifecycle != sdk::ExtensionLifecycle::inactive) {
    return ExtensionHostResult::rejected_lifecycle;
  }
  if (impl_->hardware_profiles != nullptr) {
    const auto active_profile = impl_->hardware_profiles->active_profile();
    if (!active_profile.has_value() ||
        active_profile->identity.profile_id !=
            registry_record->device_identity.hardware_profile.profile_id) {
      return ExtensionHostResult::rejected_profile;
    }
  }
  for (const auto& capability : active_capabilities) {
    if (!category_allows(registry_record->manifest.category, capability) ||
        !contains(registry_record->validated_capabilities, capability)) {
      return ExtensionHostResult::rejected_capability;
    }
  }
  if (!extensions::validate_capability_state(
           registry_record->manifest,
           {LifecycleState::active, extensions::FailureClass::none,
            registry_record->validated_capabilities, active_capabilities})
           .valid) {
    return ExtensionHostResult::rejected_capability;
  }
  for (const auto& capability : active_capabilities) {
    if (!impl_->registry.resolve(capability).empty()) {
      return ExtensionHostResult::rejected_capability;
    }
  }
  if (impl_->registry.transition(package_id, LifecycleState::activating) !=
      extensions::RegistryResult::transitioned) {
    return ExtensionHostResult::rejected_registry;
  }
  if (impl_->registry.activate_capabilities(package_id, active_capabilities) !=
      extensions::RegistryResult::capabilities_activated) {
    impl_->registry.transition(package_id, LifecycleState::inactive);
    return ExtensionHostResult::rejected_registry;
  }
  auto authority = std::make_shared<SessionAuthority>();
  authority->registry = &impl_->registry;
  authority->event_bus = &impl_->events;
  authority->package_id = package_id;
  authority->logical_device_id =
      registry_record->device_identity.logical.instance_id;
  authority->epoch = registry_record->authorization_generation;
  authority->registry_generation = registry_record->authorization_generation;
  authority->current = true;
  const auto command_session = has_command_capability(active_capabilities)
                                   ? impl_->core.bind_command_session(
                                         impl_->commands, package_id,
                                         authority->logical_device_id,
                                         authority->epoch)
                                   : api::CommandSessionResult::bound;
  if (command_session != api::CommandSessionResult::bound &&
      command_session != api::CommandSessionResult::replaced) {
    impl_->registry.transition(package_id, LifecycleState::failed,
                               extensions::FailureClass::incompatible);
    authority->current = false;
    record->lifecycle = sdk::ExtensionLifecycle::failed;
    impl_->refresh_inactive_context(*record, sdk::ExtensionLifecycle::failed);
    return ExtensionHostResult::rejected_registry;
  }
  record->authority = authority;
  record->context = impl_->make_context(
      *impl_->registry.find(package_id), sdk::ExtensionLifecycle::active,
      active_capabilities, authority);
  bool activated = false;
  try {
    activated = record->extension->activate(record->context);
  } catch (...) {
    activated = false;
  }
  if (!activated) {
    impl_->revoke(*record);
    impl_->registry.transition(package_id, LifecycleState::failed,
                               extensions::FailureClass::temporary);
    record->lifecycle = sdk::ExtensionLifecycle::failed;
    impl_->refresh_inactive_context(*record, sdk::ExtensionLifecycle::failed);
    return ExtensionHostResult::rejected_extension;
  }
  record->lifecycle = sdk::ExtensionLifecycle::active;
  return ExtensionHostResult::activated;
}

ExtensionHostResult ExtensionHost::suspend_extension(
    const std::string& package_id) {
  auto* record = impl_->find(package_id);
  if (record == nullptr) return ExtensionHostResult::rejected_not_found;
  if (record->lifecycle != sdk::ExtensionLifecycle::active) {
    return ExtensionHostResult::rejected_lifecycle;
  }
  impl_->revoke(*record);
  if (impl_->registry.transition(package_id, LifecycleState::inactive) !=
      extensions::RegistryResult::transitioned) {
    return ExtensionHostResult::rejected_registry;
  }
  notify_suspended(record->extension);
  record->lifecycle = sdk::ExtensionLifecycle::inactive;
  impl_->refresh_inactive_context(*record, sdk::ExtensionLifecycle::inactive);
  return ExtensionHostResult::suspended;
}

ExtensionHostResult ExtensionHost::fail_extension(
    const std::string& package_id, const extensions::FailureClass failure) {
  auto* record = impl_->find(package_id);
  if (record == nullptr) return ExtensionHostResult::rejected_not_found;
  if (!known_failure(failure) || failure == extensions::FailureClass::none) {
    return ExtensionHostResult::rejected_lifecycle;
  }
  impl_->revoke(*record);
  const auto* registry_record = impl_->registry.find(package_id);
  if (registry_record != nullptr &&
      registry_record->lifecycle == LifecycleState::inactive) {
    if (impl_->registry.transition(package_id, LifecycleState::activating) !=
        extensions::RegistryResult::transitioned) {
      return ExtensionHostResult::rejected_registry;
    }
  }
  if (impl_->registry.transition(package_id, LifecycleState::failed, failure) !=
      extensions::RegistryResult::transitioned) {
    return ExtensionHostResult::rejected_registry;
  }
  notify_suspended(record->extension);
  record->lifecycle = sdk::ExtensionLifecycle::failed;
  impl_->refresh_inactive_context(*record, sdk::ExtensionLifecycle::failed);
  return ExtensionHostResult::failed;
}

ExtensionHostResult ExtensionHost::quarantine_extension(
    const std::string& package_id, const extensions::FailureClass failure) {
  auto* record = impl_->find(package_id);
  if (record == nullptr) return ExtensionHostResult::rejected_not_found;
  if (!known_failure(failure) || failure == extensions::FailureClass::none) {
    return ExtensionHostResult::rejected_lifecycle;
  }
  impl_->revoke(*record);
  if (impl_->registry.transition(package_id, LifecycleState::quarantined,
                                 failure) !=
      extensions::RegistryResult::transitioned) {
    return ExtensionHostResult::rejected_registry;
  }
  notify_suspended(record->extension);
  record->lifecycle = sdk::ExtensionLifecycle::quarantined;
  impl_->refresh_inactive_context(*record, sdk::ExtensionLifecycle::quarantined);
  return ExtensionHostResult::quarantined;
}

ExtensionHostResult ExtensionHost::recover_extension(
    const std::string& package_id) {
  auto* record = impl_->find(package_id);
  const auto* registry_record = impl_->registry.find(package_id);
  if (record == nullptr || registry_record == nullptr) {
    return ExtensionHostResult::rejected_not_found;
  }
  if (record->lifecycle == sdk::ExtensionLifecycle::failed) {
    if (impl_->registry.transition(package_id, LifecycleState::inactive) !=
        extensions::RegistryResult::transitioned) {
      return ExtensionHostResult::rejected_registry;
    }
  } else if (record->lifecycle == sdk::ExtensionLifecycle::quarantined) {
    if (impl_->registry.transition(package_id, LifecycleState::disabled) !=
            extensions::RegistryResult::transitioned ||
        impl_->registry.transition(package_id, LifecycleState::inactive) !=
            extensions::RegistryResult::transitioned) {
      return ExtensionHostResult::rejected_registry;
    }
  } else {
    return ExtensionHostResult::rejected_lifecycle;
  }
  record->lifecycle = sdk::ExtensionLifecycle::inactive;
  impl_->refresh_inactive_context(*record, sdk::ExtensionLifecycle::inactive);
  return ExtensionHostResult::recovered;
}

ExtensionHostResult ExtensionHost::remove_extension(
    const std::string& package_id) {
  auto* record = impl_->find(package_id);
  if (record == nullptr) return ExtensionHostResult::rejected_not_found;
  impl_->revoke(*record);
  if (impl_->registry.unregister_extension(package_id) !=
      extensions::RegistryResult::removed) {
    return ExtensionHostResult::rejected_registry;
  }
  notify_suspended(record->extension);
  record->lifecycle = sdk::ExtensionLifecycle::removed;
  record->context.reset();
  return ExtensionHostResult::removed;
}

sdk::ExtensionLifecycle ExtensionHost::lifecycle(
    const std::string& package_id) const {
  const auto* record = impl_->find(package_id);
  return record == nullptr ? sdk::ExtensionLifecycle::removed
                           : record->lifecycle;
}

std::shared_ptr<const sdk::ExtensionContext> ExtensionHost::context(
    const std::string& package_id) const {
  auto* record = impl_->find(package_id);
  if (record == nullptr) return nullptr;
  const auto* registry_record = impl_->registry.find(package_id);
  if (registry_record == nullptr ||
      registry_record->lifecycle == LifecycleState::removed) {
    record->context.reset();
    return nullptr;
  }
  if (record->lifecycle == sdk::ExtensionLifecycle::active) {
    const bool authority_current =
        record->authority != nullptr && record->authority->current &&
        registry_record->lifecycle == LifecycleState::active &&
        registry_record->authorization_generation ==
            record->authority->registry_generation;
    if (!authority_current) {
      impl_->revoke(*record);
      switch (registry_record->lifecycle) {
        case LifecycleState::failed:
          record->lifecycle = sdk::ExtensionLifecycle::failed;
          break;
        case LifecycleState::quarantined:
          record->lifecycle = sdk::ExtensionLifecycle::quarantined;
          break;
        case LifecycleState::removed:
          record->lifecycle = sdk::ExtensionLifecycle::removed;
          record->context.reset();
          return nullptr;
        case LifecycleState::discovered:
        case LifecycleState::validating:
        case LifecycleState::installed:
        case LifecycleState::validated:
        case LifecycleState::inactive:
        case LifecycleState::activating:
        case LifecycleState::configured:
        case LifecycleState::commissioning:
        case LifecycleState::active:
        case LifecycleState::degraded:
        case LifecycleState::disabled:
          record->lifecycle = sdk::ExtensionLifecycle::inactive;
          break;
      }
      impl_->refresh_inactive_context(*record, record->lifecycle);
    }
  }
  return record->context;
}

}  // namespace zie::core
