#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "zie/devices/DeviceIdentity.hpp"
#include "zie/extensions/ExtensionRegistry.hpp"

namespace zie::core {

class ExtensionHost;

enum class ContentIdentityKind { abstract_digest };
enum class PackageTrustProvenance {
  built_in_image,
  operator_approved,
  local_developer,
  community_source,
};
enum class PackageVerificationState {
  not_checked,
  verified_trusted,
  verified_untrusted,
  invalid_signature,
  unknown_signer,
  revoked_signer,
  unsupported_algorithm,
};
enum class PackageLifecycle {
  available,
  staged,
  verified,
  compatible,
  ready,
  active,
  update_pending,
  rollback_pending,
  quarantined,
  disabled,
  removed,
};
enum class DowngradePolicy { reject, allow_authorized };

struct PackageContractRange {
  extensions::ContractVersion minimum;
  extensions::ContractVersion maximum;
};

struct ContentIdentity {
  ContentIdentityKind kind{ContentIdentityKind::abstract_digest};
  std::string value;
};

struct PackageIdentity {
  std::string package_id;
  extensions::ContractVersion package_version{};
  extensions::ContractVersion manifest_schema_version{};
  ContentIdentity content;
  std::string publisher_id;
};

// Candidate claims are never authority. Any attempt to populate these fields is
// rejected rather than merged with the external policy decision.
struct PackageAuthorityClaims {
  bool claims_trust{false};
  bool claims_controller{false};
  bool claims_profile{false};
};

struct PackageCandidate {
  PackageIdentity identity;
  extensions::ExtensionClass extension_class{
      extensions::ExtensionClass::host_plugin};
  extensions::ExtensionCategory category{
      extensions::ExtensionCategory::behavior};
  PackageContractRange sdk_contract;
  PackageContractRange core_contract;
  std::vector<std::string> requested_capabilities;
  bool contains_executable_content{true};
  PackageAuthorityClaims authority_claims;
};

struct PackageVerificationDecision {
  std::string package_id;
  std::string signer_id;
  std::string algorithm_id;
  PackageVerificationState state{PackageVerificationState::not_checked};
  extensions::TrustClass assigned_trust{
      extensions::TrustClass::community_untrusted};
  PackageTrustProvenance provenance{PackageTrustProvenance::community_source};
  devices::ControllerIdentity controller{devices::ControllerIdentity::none};
  std::string hardware_profile_id;
};

struct PackageSnapshot {
  PackageIdentity identity;
  PackageLifecycle lifecycle{PackageLifecycle::available};
  PackageVerificationState verification{PackageVerificationState::not_checked};
  extensions::TrustClass assigned_trust{
      extensions::TrustClass::community_untrusted};
  PackageTrustProvenance provenance{PackageTrustProvenance::community_source};
  devices::ControllerIdentity controller{devices::ControllerIdentity::none};
  std::string hardware_profile_id;
  std::uint64_t installation_generation{0};
};

enum class PackageResult {
  declared,
  staged,
  verified,
  compatible,
  ready,
  activated,
  update_staged,
  updated,
  rolled_back,
  quarantined,
  disabled,
  removed,
  rejected_invalid_policy,
  rejected_not_found,
  rejected_duplicate,
  rejected_identity,
  rejected_self_authority,
  rejected_unknown_domain,
  rejected_verification,
  rejected_incompatible,
  rejected_lifecycle,
  rejected_registry,
  rejected_host,
  rejected_stale_generation,
  rejected_same_version_content,
  rejected_downgrade,
  rejected_capability_expansion,
  rejected_missing_rollback,
  rejected_retry_limit,
  rejected_secret,
  rejected_generation_exhausted,
  rejected_atomic_restore,
};

class PackagePolicy {
 public:
  PackagePolicy(extensions::ExtensionRegistry& registry,
                std::size_t max_packages, std::size_t max_known_good,
                std::size_t max_attempts)
      : registry_(registry),
        max_packages_(max_packages),
        max_known_good_(max_known_good),
        max_attempts_(max_attempts) {}

  PackageResult declare_available(const PackageCandidate& candidate);
  PackageResult stage(const std::string& package_id);
  PackageResult verify(const std::string& package_id,
                       const PackageVerificationDecision& decision);
  PackageResult check_compatibility(const std::string& package_id);
  PackageResult mark_ready(const std::string& package_id);
  PackageResult confirm_active(const std::string& package_id);

  PackageResult stage_update(
      const PackageCandidate& candidate, std::uint64_t expected_generation,
      DowngradePolicy downgrade_policy,
      const std::vector<std::string>& approved_capability_expansion = {});
  PackageResult verify_update(const PackageVerificationDecision& decision);
  PackageResult check_update_compatibility(const std::string& package_id);
  PackageResult commit_update(const std::string& package_id,
                              ExtensionHost& host);
  PackageResult rollback(const std::string& package_id,
                         const std::string& target_content_identity,
                         std::uint64_t expected_generation,
                         ExtensionHost& host);
  PackageResult quarantine(const std::string& package_id,
                           ExtensionHost& host);
  PackageResult disable(const std::string& package_id, ExtensionHost& host);
  PackageResult remove(const std::string& package_id, ExtensionHost& host);

  bool allows_activation(const std::string& package_id) const;
  bool runtime_ready(const std::vector<std::string>& required_packages) const;
  std::optional<PackageSnapshot> snapshot(const std::string& package_id) const;

 private:
  struct VersionRecord {
    PackageCandidate candidate;
    PackageVerificationDecision decision;
    bool verified{false};
    bool compatible{false};
  };
  struct Record {
    VersionRecord active;
    PackageLifecycle lifecycle{PackageLifecycle::available};
    std::uint64_t installation_generation{0};
    std::optional<VersionRecord> pending;
    std::vector<std::string> approved_expansion;
    std::vector<VersionRecord> known_good;
    std::size_t update_attempts{0};
    std::size_t rollback_attempts{0};
    bool host_transition_authorized{false};
  };
  Record* find_mutable(const std::string& package_id);
  const Record* find(const std::string& package_id) const;
  bool candidate_valid(const PackageCandidate& candidate) const;
  bool decision_valid(const PackageCandidate& candidate,
                      const PackageVerificationDecision& decision) const;
  bool compatible(const PackageCandidate& candidate) const;
  bool registry_matches(const Record& record) const;
  bool next_generation(Record& record);

  extensions::ExtensionRegistry& registry_;
  std::size_t max_packages_{0};
  std::size_t max_known_good_{0};
  std::size_t max_attempts_{0};
  std::vector<Record> records_;
};

enum class SecretSurface {
  manifest,
  hardware_profile,
  runtime_checkpoint,
  log,
  event,
  package_metadata,
  plugin_configuration,
  provider_credential_handle,
};
enum class SecretPolicyResult {
  accepted,
  rejected_secret_namespace,
  rejected_secret_value,
  rejected_unknown_surface,
  rejected_cross_package_access,
};

struct OpaqueCredentialHandle {
  std::string handle_id;
  std::string owner_package_id;
  std::string provider_domain;
};

class SecretBoundaryPolicy {
 public:
  SecretPolicyResult validate(SecretSurface surface, const std::string& key,
                              const std::string& value) const;
  SecretPolicyResult authorize_handle(
      const std::string& requester_package_id,
      const OpaqueCredentialHandle& handle) const;
};

enum class WatchdogObservationDomain {
  host_runtime,
  controller_session,
  required_subsystem,
  extension,
  provider,
  package_update,
  package_rollback,
};
enum class WatchdogHealth { healthy, degraded, unavailable, failed, stuck };
enum class WatchdogReason {
  none,
  runtime_generation_stale,
  controller_liveness_lost,
  required_safety_lost,
  repeated_extension_failure,
  repeated_provider_failure,
  update_stuck,
  rollback_stuck,
  unknown_domain,
};
enum class WatchdogAction {
  none,
  degrade_runtime,
  quarantine_extension,
  fail_safe_stop_and_revoke,
  abort_update,
  abort_rollback,
};

struct WatchdogObservation {
  WatchdogObservationDomain domain{WatchdogObservationDomain::host_runtime};
  WatchdogHealth health{WatchdogHealth::healthy};
  WatchdogReason reason{WatchdogReason::none};
  std::uint64_t generation{0};
  bool required{true};
};

struct WatchdogDecision {
  WatchdogAction action{WatchdogAction::none};
  WatchdogReason reason{WatchdogReason::none};
  std::uint64_t observation_generation{0};
};

class RuntimeWatchdogPolicy {
 public:
  explicit RuntimeWatchdogPolicy(std::size_t max_repeated_failures)
      : max_repeated_failures_(max_repeated_failures) {}
  WatchdogDecision observe(const WatchdogObservation& observation);

 private:
  std::size_t max_repeated_failures_{0};
  std::size_t extension_failures_{0};
  std::size_t provider_failures_{0};
  std::uint64_t last_runtime_generation_{0};
};

}  // namespace zie::core
