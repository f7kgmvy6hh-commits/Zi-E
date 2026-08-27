#pragma once

#include <cstddef>
#include <deque>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "zie/extensions/ExtensionRegistry.hpp"

namespace zie::providers {

enum class ProviderKind { llm, stt, tts, wake };

struct LlmRequest { std::string prompt; };
struct SttRequest { std::string utterance_id; };
struct TtsRequest { std::string text; };
struct WakeRequest { std::string observation_id; };
using ProviderRequest =
    std::variant<LlmRequest, SttRequest, TtsRequest, WakeRequest>;

struct LlmResponse { std::string text; };
struct SttResponse { std::string transcript; };
struct TtsResponse { std::string utterance_id; };
struct WakeResponse { bool detected{false}; };
using ProviderResponse =
    std::variant<LlmResponse, SttResponse, TtsResponse, WakeResponse>;

enum class ProviderCallStatus { success, temporary_failure, permanent_failure };

struct ProviderCall {
  ProviderCallStatus status{ProviderCallStatus::permanent_failure};
  ProviderResponse response{LlmResponse{}};
};

class Provider {
 public:
  virtual ~Provider() = default;
  virtual ProviderKind kind() const = 0;
  virtual ProviderCall invoke(const ProviderRequest& request) = 0;
};

struct ProviderIdentity {
  std::string package_id;
  std::string logical_device_instance_id;
  std::string capability;
};

struct ProviderBinding {
  ProviderIdentity identity;
  std::shared_ptr<Provider> provider;
};

enum class ProviderResult {
  registered,
  succeeded,
  exhausted,
  rejected_invalid_router,
  rejected_invalid_binding,
  rejected_duplicate_binding,
  rejected_registry_identity,
  rejected_inactive_provider,
  rejected_missing_capability,
  rejected_category_mismatch,
  rejected_kind_mismatch,
  rejected_invalid_request,
  rejected_no_provider,
  rejected_unknown_kind,
};

struct ProviderOutcome {
  ProviderResult result{ProviderResult::exhausted};
  ProviderResponse response{LlmResponse{}};
  std::string provider_package_id;
  std::size_t attempts{0};
};

class ProviderRouter {
 public:
  ProviderRouter(const extensions::ExtensionRegistry& registry,
                 std::size_t max_providers, std::size_t max_attempts)
      : registry_(registry),
        max_providers_(max_providers),
        max_attempts_(max_attempts) {}

  ProviderResult add(const ProviderBinding& binding);
  ProviderOutcome invoke(const ProviderRequest& request);
  std::size_t provider_count() const { return bindings_.size(); }

 private:
  ProviderResult authorize(const ProviderBinding& binding) const;
  const extensions::ExtensionRegistry& registry_;
  std::size_t max_providers_{0};
  std::size_t max_attempts_{0};
  std::vector<ProviderBinding> bindings_;
};

class DeterministicMockProvider final : public Provider {
 public:
  DeterministicMockProvider(ProviderKind kind,
                            std::deque<ProviderCall> scripted_calls)
      : kind_(kind), scripted_calls_(std::move(scripted_calls)) {}

  ProviderKind kind() const override { return kind_; }
  ProviderCall invoke(const ProviderRequest& request) override;
  std::size_t call_count() const { return call_count_; }

 private:
  ProviderKind kind_;
  std::deque<ProviderCall> scripted_calls_;
  std::size_t call_count_{0};
};

}  // namespace zie::providers
