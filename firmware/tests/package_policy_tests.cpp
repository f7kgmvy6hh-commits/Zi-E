#include <cassert>

#include "core/PackagePolicy.hpp"

namespace {
using namespace zie;

core::PackageCandidate package(const std::string& content = "digest-a") {
  core::PackageCandidate value;
  value.identity = {"zie.test.package", {1, 0, 0}, {1, 0, 0},
                    {core::ContentIdentityKind::abstract_digest, content},
                    "zie.publisher"};
  value.extension_class = extensions::ExtensionClass::host_plugin;
  value.category = extensions::ExtensionCategory::behavior;
  value.sdk_contract = {{1, 0, 0}, {1, 1, 0}};
  value.core_contract = {{1, 0, 0}, {1, 0, 0}};
  value.requested_capabilities = {"events.read"};
  return value;
}

core::PackageVerificationDecision decision(
    core::PackageVerificationState state =
        core::PackageVerificationState::verified_untrusted) {
  return {"zie.test.package", "zie.signer", "abstract.algorithm", state,
          extensions::TrustClass::local_developer,
          core::PackageTrustProvenance::local_developer,
          devices::ControllerIdentity::host, "zie.test-profile"};
}

void declared_and_staged(core::PackagePolicy& policy,
                         const core::PackageCandidate& candidate = package()) {
  assert(policy.declare_available(candidate) == core::PackageResult::declared);
  assert(policy.stage(candidate.identity.package_id) ==
         core::PackageResult::staged);
}
}  // namespace

void run_package_policy_tests() {
  using namespace zie;
  {
    extensions::ExtensionRegistry registry;
    core::PackagePolicy policy(registry, 4, 2, 2);
    declared_and_staged(policy);
    assert(policy.verify("zie.test.package", decision()) ==
           core::PackageResult::verified);
    assert(policy.check_compatibility("zie.test.package") ==
           core::PackageResult::compatible);
    assert(policy.mark_ready("zie.test.package") == core::PackageResult::ready);
    assert(policy.allows_activation("zie.test.package"));
  }
  {
    extensions::ExtensionRegistry registry;
    core::PackagePolicy policy(registry, 4, 2, 2);
    auto claimed = package();
    claimed.authority_claims.claims_trust = true;
    assert(policy.declare_available(claimed) ==
           core::PackageResult::rejected_self_authority);
    auto asset = package();
    asset.identity.package_id = "zie.test.asset";
    asset.extension_class = extensions::ExtensionClass::asset_pack;
    assert(policy.declare_available(asset) ==
           core::PackageResult::rejected_identity);
  }
  for (const auto state : {core::PackageVerificationState::invalid_signature,
                           core::PackageVerificationState::unknown_signer,
                           core::PackageVerificationState::revoked_signer,
                           core::PackageVerificationState::unsupported_algorithm}) {
    extensions::ExtensionRegistry registry;
    core::PackagePolicy policy(registry, 2, 1, 1);
    declared_and_staged(policy);
    assert(policy.verify("zie.test.package", decision(state)) ==
           core::PackageResult::rejected_verification);
  }
  {
    extensions::ExtensionRegistry registry;
    core::PackagePolicy policy(registry, 2, 1, 1);
    declared_and_staged(policy);
    auto unknown = decision(static_cast<core::PackageVerificationState>(255));
    assert(policy.verify("zie.test.package", unknown) ==
           core::PackageResult::rejected_unknown_domain);
  }
  {
    core::SecretBoundaryPolicy secrets;
    assert(secrets.validate(core::SecretSurface::plugin_configuration,
                            "secret.token", "opaque") ==
           core::SecretPolicyResult::rejected_secret_namespace);
    const core::OpaqueCredentialHandle handle{"credential-1", "zie.owner",
                                               "llm"};
    assert(secrets.authorize_handle("zie.other", handle) ==
           core::SecretPolicyResult::rejected_cross_package_access);
    assert(secrets.authorize_handle("zie.owner", handle) ==
           core::SecretPolicyResult::accepted);
  }
  {
    core::RuntimeWatchdogPolicy watchdog(2);
    assert(watchdog.observe({core::WatchdogObservationDomain::required_subsystem,
                             core::WatchdogHealth::failed,
                             core::WatchdogReason::required_safety_lost, 1,
                             true}).action ==
           core::WatchdogAction::fail_safe_stop_and_revoke);
    assert(watchdog.observe({core::WatchdogObservationDomain::provider,
                             core::WatchdogHealth::failed,
                             core::WatchdogReason::repeated_provider_failure, 2,
                             false}).action ==
           core::WatchdogAction::degrade_runtime);
    assert(watchdog.observe({core::WatchdogObservationDomain::extension,
                             core::WatchdogHealth::failed,
                             core::WatchdogReason::repeated_extension_failure,
                             3, false}).action ==
           core::WatchdogAction::degrade_runtime);
    assert(watchdog.observe({core::WatchdogObservationDomain::extension,
                             core::WatchdogHealth::failed,
                             core::WatchdogReason::repeated_extension_failure,
                             4, false}).action ==
           core::WatchdogAction::quarantine_extension);
    assert(watchdog.observe(
               {static_cast<core::WatchdogObservationDomain>(255),
                core::WatchdogHealth::healthy, core::WatchdogReason::none, 5,
                true}).action ==
           core::WatchdogAction::fail_safe_stop_and_revoke);
  }
}
