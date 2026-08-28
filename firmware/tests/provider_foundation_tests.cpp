#include <cassert>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>

#include "zie/providers/ProviderFoundation.hpp"

namespace {
using namespace zie::extensions;
using namespace zie::providers;
using zie::devices::ControllerIdentity;

ExtensionCandidate candidate(const std::string& id, const std::string& logical,
                             const std::string& serial,
                             const ExtensionCategory category,
                             const std::string& capability) {
  ExtensionCandidate value;
  value.manifest.id = id;
  value.manifest.name = id;
  value.manifest.version = {1, 0, 0};
  value.manifest.plugin_api_version = supported_plugin_api_version();
  value.manifest.manifest_schema_version = supported_manifest_schema_version();
  value.manifest.category = category;
  value.manifest.extension_class = ExtensionClass::host_plugin;
  value.manifest.controller_target = ControllerTarget::host;
  value.manifest.entrypoint = "create_mock_provider";
  value.manifest.declared_capabilities = {capability};
  value.device_identity.package.extension_id = id;
  value.device_identity.physical.state =
      zie::devices::PhysicalIdentityState::manufacturer_serial;
  value.device_identity.physical.manufacturer = "ZI-E Test";
  value.device_identity.physical.model = "Provider Fixture";
  value.device_identity.physical.serial = serial;
  value.device_identity.physical.provenance = {
      zie::devices::IdentitySource::operator_verified,
      zie::devices::IdentityTrust::verified};
  value.device_identity.logical.instance_id = logical;
  value.device_identity.controller.controller = ControllerIdentity::host;
  value.device_identity.hardware_profile.profile_id = "zie.test-profile";
  return value;
}

void activate(ExtensionRegistry& registry, const ExtensionCandidate& value) {
  assert(registry.register_extension(
             value, {value.manifest.id, TrustClass::local_developer,
                     ControllerIdentity::host, "zie.test-profile", {}}) ==
         RegistryResult::accepted);
  assert(registry.transition(value.manifest.id, LifecycleState::validated) ==
         RegistryResult::transitioned);
  assert(registry.validate_capabilities(
             value.manifest.id, value.manifest.declared_capabilities) ==
         RegistryResult::capabilities_validated);
  assert(registry.transition(value.manifest.id, LifecycleState::inactive) ==
         RegistryResult::transitioned);
  assert(registry.transition(value.manifest.id, LifecycleState::activating) ==
         RegistryResult::transitioned);
  assert(registry.activate_capabilities(
             value.manifest.id, value.manifest.declared_capabilities) ==
         RegistryResult::capabilities_activated);
}

ProviderBinding binding(const ExtensionCandidate& value,
                        const std::shared_ptr<Provider>& provider,
                        const std::string& semantic_capability) {
  return {{value.manifest.id, value.device_identity.logical.instance_id,
           value.manifest.declared_capabilities.front(), semantic_capability},
          provider};
}

class ThrowingProvider final : public Provider {
 public:
  ProviderKind kind() const override { return ProviderKind::llm; }
  ProviderCall invoke(const ProviderRequest&) override {
    throw std::runtime_error("adversarial provider");
  }
};

class RevokingProvider final : public Provider {
 public:
  RevokingProvider(ExtensionRegistry& registry, std::string target)
      : registry_(registry), target_(std::move(target)) {}
  ProviderKind kind() const override { return ProviderKind::llm; }
  ProviderCall invoke(const ProviderRequest&) override {
    const auto result = registry_.transition(target_, LifecycleState::quarantined,
                                             FailureClass::security);
    assert(result == RegistryResult::transitioned);
    return {ProviderCallStatus::temporary_failure, LlmResponse{}};
  }
 private:
  ExtensionRegistry& registry_;
  std::string target_;
};
}  // namespace

void run_provider_foundation_tests() {
  ExtensionRegistry registry;
  const auto llm_bad = candidate("zie.mock.llm-bad", "provider.llm.bad",
                                 "provider-001", ExtensionCategory::ai_provider,
                                 "provider.llm.mock-bad");
  const auto llm_good = candidate("zie.mock.llm-good", "provider.llm.good",
                                  "provider-002", ExtensionCategory::ai_provider,
                                  "provider.llm.mock-good");
  const auto stt = candidate("zie.mock.stt", "provider.stt.primary",
                             "provider-003", ExtensionCategory::voice_stt,
                             "provider.stt.mock");
  const auto tts = candidate("zie.mock.tts", "provider.tts.primary",
                             "provider-004", ExtensionCategory::voice_tts,
                             "provider.tts.mock");
  const auto wake = candidate("zie.mock.wake", "provider.wake.primary",
                              "provider-005", ExtensionCategory::voice_wakeword,
                              "provider.wake.mock");
  activate(registry, llm_bad);
  activate(registry, llm_good);
  activate(registry, stt);
  activate(registry, tts);
  activate(registry, wake);

  auto temporary = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm, std::deque<ProviderCall>{{
          ProviderCallStatus::temporary_failure, LlmResponse{}}});
  auto good = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm, std::deque<ProviderCall>{{
          ProviderCallStatus::success, LlmResponse{"deterministic reply"}}});
  auto stt_mock = std::make_shared<DeterministicMockProvider>(
      ProviderKind::stt, std::deque<ProviderCall>{{
          ProviderCallStatus::success,
          SttResponse{"deterministic transcript"}}});
  auto tts_mock = std::make_shared<DeterministicMockProvider>(
      ProviderKind::tts, std::deque<ProviderCall>{{
          ProviderCallStatus::success, TtsResponse{"utterance.mock.1"}}});
  auto wake_mock = std::make_shared<DeterministicMockProvider>(
      ProviderKind::wake, std::deque<ProviderCall>{{
          ProviderCallStatus::success, WakeResponse{true}}});

  ProviderRouter router(registry, 5, 2, 5);
  assert(router.add(binding(llm_bad, temporary, "provider.llm.chat")) == ProviderResult::registered);
  assert(router.add(binding(llm_good, good, "provider.llm.chat")) == ProviderResult::registered);
  assert(router.add(binding(stt, stt_mock, "provider.stt.transcribe")) == ProviderResult::registered);
  assert(router.add(binding(tts, tts_mock, "provider.tts.speak")) == ProviderResult::registered);
  assert(router.add(binding(wake, wake_mock, "provider.wake.detect")) == ProviderResult::registered);
  const auto llm_outcome = router.invoke({"provider.llm.chat", LlmRequest{"hello"}});
  assert(llm_outcome.result == ProviderResult::succeeded);
  assert(llm_outcome.attempts == 2);
  assert(llm_outcome.provider_package_id == llm_good.manifest.id);
  assert(llm_outcome.diagnostics.size() == 1);
  assert(llm_outcome.diagnostics.front().failure == ProviderFailure::temporary_failure);
  assert(llm_outcome.last_failure == ProviderFailure::temporary_failure);
  assert(std::get<LlmResponse>(llm_outcome.response).text ==
         "deterministic reply");
  assert(router.invoke({"provider.stt.transcribe", SttRequest{"utterance.input"}}).result ==
         ProviderResult::succeeded);
  assert(router.invoke({"provider.tts.speak", TtsRequest{"speak"}}).result == ProviderResult::succeeded);
  assert(std::get<WakeResponse>(router.invoke({"provider.wake.detect", WakeRequest{"wake.input"}}).response)
             .detected);

  ProviderRouter prioritized(registry, 5, 3, 5);
  auto preferred = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm,
      std::deque<ProviderCall>{
          {ProviderCallStatus::temporary_failure, LlmResponse{}},
          {ProviderCallStatus::success, LlmResponse{"primary-restored"}}});
  auto secondary = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm,
      std::deque<ProviderCall>{{ProviderCallStatus::success,
                                LlmResponse{"bounded-fallback"}}});
  auto prioritized_stt = std::make_shared<DeterministicMockProvider>(
      ProviderKind::stt, std::deque<ProviderCall>{});
  assert(prioritized.add(binding(llm_bad, preferred, "provider.llm.chat")) ==
         ProviderResult::registered);
  assert(prioritized.add(binding(llm_good, secondary, "provider.llm.chat")) ==
         ProviderResult::registered);
  assert(prioritized.add(
             binding(stt, prioritized_stt, "provider.stt.transcribe")) ==
         ProviderResult::registered);
  assert(prioritized.configure_priority(
             {ProviderKind::llm, "provider.llm.chat",
              {llm_bad.manifest.id, llm_good.manifest.id}, 1},
             0) == ProviderResult::priority_configured);
  assert(prioritized.priority_generation() == 1);
  assert(prioritized.configure_priority(
             {ProviderKind::stt, "provider.stt.transcribe",
              {stt.manifest.id}, 1},
             0) == ProviderResult::rejected_priority_generation);
  assert(prioritized.configure_priority(
             {ProviderKind::stt, "provider.stt.transcribe",
              {stt.manifest.id}, 1},
             1) == ProviderResult::priority_configured);
  assert(prioritized.configure_priority(
             {ProviderKind::llm, "provider.stt.transcribe",
              {stt.manifest.id}, 2},
             2) == ProviderResult::rejected_invalid_priority);
  assert(prioritized.configure_priority(
             {static_cast<ProviderKind>(255), "provider.llm.chat",
              {llm_bad.manifest.id}, 2},
             2) == ProviderResult::rejected_invalid_priority);
  const auto fallback_result = prioritized.invoke(
      {"provider.llm.chat", LlmRequest{"first"}});
  assert(fallback_result.result == ProviderResult::succeeded);
  assert(fallback_result.provider_package_id == llm_good.manifest.id);
  const auto primary_retry = prioritized.invoke(
      {"provider.llm.chat", LlmRequest{"second"}});
  assert(primary_retry.result == ProviderResult::succeeded);
  assert(primary_retry.provider_package_id == llm_bad.manifest.id);
  assert(preferred->call_count() == 2 && secondary->call_count() == 1);

  ProviderRouter bounded(registry, 2, 1, 2);
  auto first_failure = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm, std::deque<ProviderCall>{{
          ProviderCallStatus::temporary_failure, LlmResponse{}}});
  auto forbidden_second = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm, std::deque<ProviderCall>{{
          ProviderCallStatus::success, LlmResponse{"must-not-run"}}});
  assert(bounded.add(binding(llm_bad, first_failure, "provider.llm.chat")) ==
         ProviderResult::registered);
  assert(bounded.add(binding(llm_good, forbidden_second, "provider.llm.chat")) ==
         ProviderResult::registered);
  const auto bounded_outcome = bounded.invoke({"provider.llm.chat", LlmRequest{"bounded"}});
  assert(bounded_outcome.result == ProviderResult::exhausted);
  assert(bounded_outcome.attempts == 1);
  assert(bounded_outcome.last_failure == ProviderFailure::temporary_failure);
  assert(forbidden_second->call_count() == 0);

  ProviderRouter adversarial(registry, 3, 3, 3);
  auto throwing = std::make_shared<ThrowingProvider>();
  auto malformed = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm, std::deque<ProviderCall>{{
          ProviderCallStatus::success, SttResponse{"wrong response kind"}}});
  auto final_good = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm, std::deque<ProviderCall>{{
          ProviderCallStatus::success, LlmResponse{"recovered"}}});
  assert(adversarial.add(binding(llm_bad, throwing, "provider.llm.chat")) ==
         ProviderResult::registered);
  assert(adversarial.add(binding(llm_good, malformed, "provider.llm.chat")) ==
         ProviderResult::registered);
  const auto third = candidate("zie.mock.llm-third", "provider.llm.third",
                               "provider-006", ExtensionCategory::ai_provider,
                               "provider.llm.mock-third");
  activate(registry, third);
  assert(adversarial.add(binding(third, final_good, "provider.llm.chat")) ==
         ProviderResult::registered);
  const auto recovered = adversarial.invoke({"provider.llm.chat", LlmRequest{"recover"}});
  assert(recovered.result == ProviderResult::succeeded);
  assert(recovered.attempts == 3);
  assert(recovered.diagnostics.size() == 2);
  assert(recovered.diagnostics[0].failure == ProviderFailure::provider_exception);
  assert(recovered.diagnostics[1].failure == ProviderFailure::malformed_response);

  ProviderRouter permanent_router(registry, 1, 1, 1);
  auto permanent = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm, std::deque<ProviderCall>{{
          ProviderCallStatus::permanent_failure, LlmResponse{}}});
  assert(permanent_router.add(binding(llm_bad, permanent,
                                      "provider.llm.chat")) ==
         ProviderResult::registered);
  const auto permanent_outcome = permanent_router.invoke(
      {"provider.llm.chat", LlmRequest{"permanent"}});
  assert(permanent_outcome.result == ProviderResult::exhausted);
  assert(permanent_outcome.last_failure == ProviderFailure::permanent_failure);

  ProviderRouter mismatch_router(registry, 3, 3, 2);
  auto mismatch_one = std::make_shared<DeterministicMockProvider>(ProviderKind::llm, std::deque<ProviderCall>{});
  auto mismatch_two = std::make_shared<DeterministicMockProvider>(ProviderKind::llm, std::deque<ProviderCall>{});
  auto mismatch_three = std::make_shared<DeterministicMockProvider>(ProviderKind::llm, std::deque<ProviderCall>{});
  assert(mismatch_router.add(binding(llm_bad, mismatch_one, "provider.llm.alpha")) == ProviderResult::registered);
  assert(mismatch_router.add(binding(llm_good, mismatch_two, "provider.llm.beta")) == ProviderResult::registered);
  assert(mismatch_router.add(binding(third, mismatch_three, "provider.llm.gamma")) == ProviderResult::registered);
  const auto mismatch_outcome = mismatch_router.invoke({"provider.llm.other", LlmRequest{"wrong-capability"}});
  assert(mismatch_outcome.result == ProviderResult::rejected_no_provider);
  assert(mismatch_outcome.last_failure == ProviderFailure::capability_mismatch);
  assert(mismatch_outcome.diagnostics.size() == 2);
  assert(mismatch_outcome.diagnostics_truncated);
  assert(mismatch_one->call_count() == 0 && mismatch_two->call_count() == 0 && mismatch_three->call_count() == 0);

  const auto revocable = candidate("zie.mock.llm-revocable", "provider.llm.revocable",
                                   "provider-007", ExtensionCategory::ai_provider,
                                   "provider.llm.mock-revocable");
  activate(registry, revocable);
  ProviderRouter revoke_path(registry, 2, 2, 2);
  auto revoker = std::make_shared<RevokingProvider>(registry, revocable.manifest.id);
  auto must_not_run = std::make_shared<DeterministicMockProvider>(ProviderKind::llm,
      std::deque<ProviderCall>{{ProviderCallStatus::success, LlmResponse{"unsafe"}}});
  assert(revoke_path.add(binding(llm_bad, revoker, "provider.llm.chat")) == ProviderResult::registered);
  assert(revoke_path.add(binding(revocable, must_not_run, "provider.llm.chat")) == ProviderResult::registered);
  const auto revoked_path = revoke_path.invoke({"provider.llm.chat", LlmRequest{"revoke"}});
  assert(revoked_path.result == ProviderResult::exhausted);
  assert(revoked_path.last_failure == ProviderFailure::authorization_loss);
  assert(revoked_path.diagnostics.size() == 2);
  assert(!revoked_path.diagnostics.back().provider_called);
  assert(must_not_run->call_count() == 0);

  ProviderRouter rejection(registry, 2, 2, 2);
  assert(rejection.add(binding(stt, good, "provider.stt.transcribe")) ==
         ProviderResult::rejected_kind_mismatch);
  auto wrong_identity = binding(llm_good, good, "provider.llm.chat");
  wrong_identity.identity.logical_device_instance_id = "impersonated";
  assert(rejection.add(wrong_identity) ==
         ProviderResult::rejected_registry_identity);
  auto missing_capability = binding(llm_good, good, "provider.llm.chat");
  missing_capability.identity.registry_capability = "provider.llm.not-active";
  assert(rejection.add(missing_capability) ==
         ProviderResult::rejected_missing_capability);
  assert(rejection.invoke({"provider.llm.chat", LlmRequest{"no provider"}}).result ==
         ProviderResult::rejected_no_provider);
  assert(router.invoke({"provider.llm.chat", LlmRequest{""}}).result ==
         ProviderResult::rejected_invalid_request);

  assert(registry.transition(llm_good.manifest.id, LifecycleState::quarantined,
                             FailureClass::security) ==
         RegistryResult::transitioned);
  ProviderRouter revoked(registry, 1, 1, 1);
  assert(revoked.add(binding(llm_good, good, "provider.llm.chat")) ==
         ProviderResult::rejected_inactive_provider);
}
