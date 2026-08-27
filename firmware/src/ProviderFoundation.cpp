#include "zie/providers/ProviderFoundation.hpp"

#include <algorithm>
#include <exception>
#include <type_traits>

namespace zie::providers {
namespace {

bool known(const ProviderKind kind) {
  switch (kind) {
    case ProviderKind::llm:
    case ProviderKind::stt:
    case ProviderKind::tts:
    case ProviderKind::wake:
      return true;
  }
  return false;
}

ProviderKind request_kind(const ProviderRequest& request) {
  if (std::holds_alternative<LlmRequest>(request)) return ProviderKind::llm;
  if (std::holds_alternative<SttRequest>(request)) return ProviderKind::stt;
  if (std::holds_alternative<TtsRequest>(request)) return ProviderKind::tts;
  return ProviderKind::wake;
}

bool valid_request(const ProviderRequest& request) {
  return std::visit([](const auto& value) {
    using T = std::decay_t<decltype(value)>;
    if constexpr (std::is_same_v<T, LlmRequest>) {
      return !value.prompt.empty();
    } else if constexpr (std::is_same_v<T, SttRequest>) {
      return !value.utterance_id.empty();
    } else if constexpr (std::is_same_v<T, TtsRequest>) {
      return !value.text.empty();
    } else {
      return !value.observation_id.empty();
    }
  }, request);
}

bool matching_response(const ProviderKind kind, const ProviderResponse& response) {
  switch (kind) {
    case ProviderKind::llm: return std::holds_alternative<LlmResponse>(response);
    case ProviderKind::stt: return std::holds_alternative<SttResponse>(response);
    case ProviderKind::tts: return std::holds_alternative<TtsResponse>(response);
    case ProviderKind::wake: return std::holds_alternative<WakeResponse>(response);
  }
  return false;
}

bool valid_response(const ProviderKind kind, const ProviderResponse& response) {
  if (!matching_response(kind, response)) return false;
  if (kind == ProviderKind::wake) return true;
  return std::visit([](const auto& value) {
    using T = std::decay_t<decltype(value)>;
    if constexpr (std::is_same_v<T, LlmResponse>) {
      return !value.text.empty();
    } else if constexpr (std::is_same_v<T, SttResponse>) {
      return !value.transcript.empty();
    } else if constexpr (std::is_same_v<T, TtsResponse>) {
      return !value.utterance_id.empty();
    } else {
      return true;
    }
  }, response);
}

std::string prefix(const ProviderKind kind) {
  switch (kind) {
    case ProviderKind::llm: return "provider.llm.";
    case ProviderKind::stt: return "provider.stt.";
    case ProviderKind::tts: return "provider.tts.";
    case ProviderKind::wake: return "provider.wake.";
  }
  return {};
}

extensions::ExtensionCategory category(const ProviderKind kind) {
  switch (kind) {
    case ProviderKind::llm: return extensions::ExtensionCategory::ai_provider;
    case ProviderKind::stt: return extensions::ExtensionCategory::voice_stt;
    case ProviderKind::tts: return extensions::ExtensionCategory::voice_tts;
    case ProviderKind::wake: return extensions::ExtensionCategory::voice_wakeword;
  }
  return extensions::ExtensionCategory::behavior;
}

}  // namespace

ProviderResult ProviderRouter::authorize(const ProviderBinding& binding) const {
  if (!binding.provider || binding.identity.package_id.empty() ||
      binding.identity.logical_device_instance_id.empty() ||
      binding.identity.capability.empty()) {
    return ProviderResult::rejected_invalid_binding;
  }
  const auto kind = binding.provider->kind();
  if (!known(kind)) return ProviderResult::rejected_unknown_kind;
  if (binding.identity.capability.rfind(prefix(kind), 0) != 0) {
    return ProviderResult::rejected_kind_mismatch;
  }
  const auto* record = registry_.find(binding.identity.package_id);
  if (record == nullptr || record->device_identity.logical.instance_id !=
                               binding.identity.logical_device_instance_id) {
    return ProviderResult::rejected_registry_identity;
  }
  if (record->manifest.category != category(kind)) {
    return ProviderResult::rejected_category_mismatch;
  }
  if (record->lifecycle != extensions::LifecycleState::active) {
    return ProviderResult::rejected_inactive_provider;
  }
  if (std::find(record->active_capabilities.begin(),
                record->active_capabilities.end(),
                binding.identity.capability) ==
      record->active_capabilities.end()) {
    return ProviderResult::rejected_missing_capability;
  }
  return ProviderResult::registered;
}

ProviderResult ProviderRouter::add(const ProviderBinding& binding) {
  if (max_providers_ == 0 || max_attempts_ == 0) {
    return ProviderResult::rejected_invalid_router;
  }
  if (bindings_.size() >= max_providers_) {
    return ProviderResult::rejected_invalid_binding;
  }
  const auto authorization = authorize(binding);
  if (authorization != ProviderResult::registered) return authorization;
  const auto duplicate = std::find_if(
      bindings_.begin(), bindings_.end(), [&binding](const ProviderBinding& value) {
        return value.identity.package_id == binding.identity.package_id ||
               value.identity.capability == binding.identity.capability;
      });
  if (duplicate != bindings_.end()) {
    return ProviderResult::rejected_duplicate_binding;
  }
  bindings_.push_back(binding);
  return ProviderResult::registered;
}

ProviderOutcome ProviderRouter::invoke(const ProviderRequest& request) {
  if (max_providers_ == 0 || max_attempts_ == 0) {
    return {ProviderResult::rejected_invalid_router, LlmResponse{}, {}, 0};
  }
  if (!valid_request(request)) {
    return {ProviderResult::rejected_invalid_request, LlmResponse{}, {}, 0};
  }
  const auto kind = request_kind(request);
  bool matched = false;
  bool eligible = false;
  ProviderOutcome outcome;
  for (const auto& binding : bindings_) {
    if (binding.provider->kind() != kind) continue;
    matched = true;
    if (authorize(binding) != ProviderResult::registered) continue;
    eligible = true;
    if (outcome.attempts >= max_attempts_) break;
    ++outcome.attempts;
    try {
      const auto call = binding.provider->invoke(request);
      if (call.status == ProviderCallStatus::success &&
          valid_response(kind, call.response)) {
        outcome.result = ProviderResult::succeeded;
        outcome.response = call.response;
        outcome.provider_package_id = binding.identity.package_id;
        return outcome;
      }
    } catch (const std::exception&) {
    } catch (...) {
    }
  }
  if (!matched || !eligible) {
    outcome.result = ProviderResult::rejected_no_provider;
    return outcome;
  }
  outcome.result = ProviderResult::exhausted;
  return outcome;
}

ProviderCall DeterministicMockProvider::invoke(const ProviderRequest& request) {
  ++call_count_;
  if (request_kind(request) != kind_ || scripted_calls_.empty()) {
    return {ProviderCallStatus::permanent_failure, LlmResponse{}};
  }
  const auto call = scripted_calls_.front();
  scripted_calls_.pop_front();
  return call;
}

}  // namespace zie::providers
