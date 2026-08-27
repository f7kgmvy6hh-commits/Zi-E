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
                     ControllerIdentity::host, "zie.test-profile"}) ==
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
                        const std::shared_ptr<Provider>& provider) {
  return {{value.manifest.id, value.device_identity.logical.instance_id,
           value.manifest.declared_capabilities.front()}, provider};
}

class ThrowingProvider final : public Provider {
 public:
  ProviderKind kind() const override { return ProviderKind::llm; }
  ProviderCall invoke(const ProviderRequest&) override {
    throw std::runtime_error("adversarial provider");
  }
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

  ProviderRouter router(registry, 5, 2);
  assert(router.add(binding(llm_bad, temporary)) == ProviderResult::registered);
  assert(router.add(binding(llm_good, good)) == ProviderResult::registered);
  assert(router.add(binding(stt, stt_mock)) == ProviderResult::registered);
  assert(router.add(binding(tts, tts_mock)) == ProviderResult::registered);
  assert(router.add(binding(wake, wake_mock)) == ProviderResult::registered);
  const auto llm_outcome = router.invoke(LlmRequest{"hello"});
  assert(llm_outcome.result == ProviderResult::succeeded);
  assert(llm_outcome.attempts == 2);
  assert(llm_outcome.provider_package_id == llm_good.manifest.id);
  assert(std::get<LlmResponse>(llm_outcome.response).text ==
         "deterministic reply");
  assert(router.invoke(SttRequest{"utterance.input"}).result ==
         ProviderResult::succeeded);
  assert(router.invoke(TtsRequest{"speak"}).result == ProviderResult::succeeded);
  assert(std::get<WakeResponse>(router.invoke(WakeRequest{"wake.input"}).response)
             .detected);

  ProviderRouter bounded(registry, 2, 1);
  auto first_failure = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm, std::deque<ProviderCall>{{
          ProviderCallStatus::temporary_failure, LlmResponse{}}});
  auto forbidden_second = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm, std::deque<ProviderCall>{{
          ProviderCallStatus::success, LlmResponse{"must-not-run"}}});
  assert(bounded.add(binding(llm_bad, first_failure)) ==
         ProviderResult::registered);
  assert(bounded.add(binding(llm_good, forbidden_second)) ==
         ProviderResult::registered);
  const auto bounded_outcome = bounded.invoke(LlmRequest{"bounded"});
  assert(bounded_outcome.result == ProviderResult::exhausted);
  assert(bounded_outcome.attempts == 1);
  assert(forbidden_second->call_count() == 0);

  ProviderRouter adversarial(registry, 3, 3);
  auto throwing = std::make_shared<ThrowingProvider>();
  auto malformed = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm, std::deque<ProviderCall>{{
          ProviderCallStatus::success, SttResponse{"wrong response kind"}}});
  auto final_good = std::make_shared<DeterministicMockProvider>(
      ProviderKind::llm, std::deque<ProviderCall>{{
          ProviderCallStatus::success, LlmResponse{"recovered"}}});
  assert(adversarial.add(binding(llm_bad, throwing)) ==
         ProviderResult::registered);
  assert(adversarial.add(binding(llm_good, malformed)) ==
         ProviderResult::registered);
  const auto third = candidate("zie.mock.llm-third", "provider.llm.third",
                               "provider-006", ExtensionCategory::ai_provider,
                               "provider.llm.mock-third");
  activate(registry, third);
  assert(adversarial.add(binding(third, final_good)) ==
         ProviderResult::registered);
  const auto recovered = adversarial.invoke(LlmRequest{"recover"});
  assert(recovered.result == ProviderResult::succeeded);
  assert(recovered.attempts == 3);

  ProviderRouter rejection(registry, 2, 2);
  assert(rejection.add(binding(stt, good)) ==
         ProviderResult::rejected_kind_mismatch);
  auto wrong_identity = binding(llm_good, good);
  wrong_identity.identity.logical_device_instance_id = "impersonated";
  assert(rejection.add(wrong_identity) ==
         ProviderResult::rejected_registry_identity);
  auto missing_capability = binding(llm_good, good);
  missing_capability.identity.capability = "provider.llm.not-active";
  assert(rejection.add(missing_capability) ==
         ProviderResult::rejected_missing_capability);
  assert(rejection.invoke(LlmRequest{"no provider"}).result ==
         ProviderResult::rejected_no_provider);
  assert(router.invoke(LlmRequest{""}).result ==
         ProviderResult::rejected_invalid_request);

  assert(registry.transition(llm_good.manifest.id, LifecycleState::quarantined,
                             FailureClass::security) ==
         RegistryResult::transitioned);
  ProviderRouter revoked(registry, 1, 1);
  assert(revoked.add(binding(llm_good, good)) ==
         ProviderResult::rejected_inactive_provider);
}
