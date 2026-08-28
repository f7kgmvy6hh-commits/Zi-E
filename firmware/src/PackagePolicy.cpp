#include "core/PackagePolicy.hpp"

#include <algorithm>
#include <cctype>
#include <limits>

#include "core/ExtensionHost.hpp"
#include "zie/sdk/PluginSdk.hpp"

namespace zie::core {
namespace {

bool valid_key(const std::string& value) {
  if (value.empty() ||
      !std::isalnum(static_cast<unsigned char>(value.front()))) {
    return false;
  }
  return std::all_of(value.begin(), value.end(), [](const unsigned char part) {
    return std::islower(part) || std::isdigit(part) || part == '.' ||
           part == '_' || part == '-';
  });
}

int compare_version(const extensions::ContractVersion left,
                    const extensions::ContractVersion right) {
  if (left.major != right.major) return left.major < right.major ? -1 : 1;
  if (left.minor != right.minor) return left.minor < right.minor ? -1 : 1;
  if (left.patch != right.patch) return left.patch < right.patch ? -1 : 1;
  return 0;
}

bool range_contains(const PackageContractRange& range,
                    const extensions::ContractVersion value) {
  return compare_version(range.minimum, range.maximum) <= 0 &&
         compare_version(range.minimum, value) <= 0 &&
         compare_version(value, range.maximum) <= 0;
}

bool known(const ContentIdentityKind value) {
  switch (value) {
    case ContentIdentityKind::abstract_digest:
      return true;
  }
  return false;
}

bool known(const PackageTrustProvenance value) {
  switch (value) {
    case PackageTrustProvenance::built_in_image:
    case PackageTrustProvenance::operator_approved:
    case PackageTrustProvenance::local_developer:
    case PackageTrustProvenance::community_source:
      return true;
  }
  return false;
}

bool known(const PackageVerificationState value) {
  switch (value) {
    case PackageVerificationState::not_checked:
    case PackageVerificationState::verified_trusted:
    case PackageVerificationState::verified_untrusted:
    case PackageVerificationState::invalid_signature:
    case PackageVerificationState::unknown_signer:
    case PackageVerificationState::revoked_signer:
    case PackageVerificationState::unsupported_algorithm:
      return true;
  }
  return false;
}

bool known(const DowngradePolicy value) {
  switch (value) {
    case DowngradePolicy::reject:
    case DowngradePolicy::allow_authorized:
      return true;
  }
  return false;
}

bool known(const extensions::ExtensionClass value) {
  switch (value) {
    case extensions::ExtensionClass::host_plugin:
    case extensions::ExtensionClass::asset_pack:
    case extensions::ExtensionClass::embedded_module:
    case extensions::ExtensionClass::protected_safety_module:
      return true;
  }
  return false;
}

bool known(const extensions::ExtensionCategory value) {
  return static_cast<unsigned>(value) <=
         static_cast<unsigned>(extensions::ExtensionCategory::integration);
}

bool known(const extensions::TrustClass value) {
  switch (value) {
    case extensions::TrustClass::built_in:
    case extensions::TrustClass::signed_trusted:
    case extensions::TrustClass::local_developer:
    case extensions::TrustClass::community_untrusted:
      return true;
  }
  return false;
}

bool known(const devices::ControllerIdentity value) {
  switch (value) {
    case devices::ControllerIdentity::none:
    case devices::ControllerIdentity::host:
    case devices::ControllerIdentity::esp32_s3:
    case devices::ControllerIdentity::stm32_safety:
      return true;
  }
  return false;
}

bool known(const WatchdogHealth value) {
  switch (value) {
    case WatchdogHealth::healthy:
    case WatchdogHealth::degraded:
    case WatchdogHealth::unavailable:
    case WatchdogHealth::failed:
    case WatchdogHealth::stuck:
      return true;
  }
  return false;
}

bool accepted_verification(const PackageVerificationState value) {
  return value == PackageVerificationState::verified_trusted ||
         value == PackageVerificationState::verified_untrusted;
}

bool contains(const std::vector<std::string>& values,
              const std::string& value) {
  return std::find(values.begin(), values.end(), value) != values.end();
}

bool secret_token(const std::string& value) {
  static const std::vector<std::string> tokens{
      "secret", "password", "passwd", "api-key", "api_key",
      "private-key", "private_key", "access-token", "access_token",
      "credential", "bearer"};
  std::string normalized;
  normalized.reserve(value.size());
  for (const unsigned char part : value) {
    if (std::isalnum(part)) {
      normalized.push_back(static_cast<char>(std::tolower(part)));
    } else {
      normalized.push_back('_');
    }
  }
  return std::any_of(tokens.begin(), tokens.end(),
                     [&normalized](std::string token) {
                       std::replace(token.begin(), token.end(), '-', '_');
                       return normalized.find(token) != std::string::npos;
                     });
}

bool ephemeral_identity(const std::string& value) {
  return value.find('/') != std::string::npos ||
         value.find('\\') != std::string::npos ||
         value.find("://") != std::string::npos;
}

}  // namespace

PackagePolicy::Record* PackagePolicy::find_mutable(
    const std::string& package_id) {
  const auto found = std::find_if(records_.begin(), records_.end(),
                                  [&package_id](const Record& record) {
                                    return record.active.candidate.identity
                                               .package_id == package_id;
                                  });
  return found == records_.end() ? nullptr : &*found;
}

const PackagePolicy::Record* PackagePolicy::find(
    const std::string& package_id) const {
  const auto found = std::find_if(records_.begin(), records_.end(),
                                  [&package_id](const Record& record) {
                                    return record.active.candidate.identity
                                               .package_id == package_id;
                                  });
  return found == records_.end() ? nullptr : &*found;
}

bool PackagePolicy::candidate_valid(const PackageCandidate& candidate) const {
  if (!valid_key(candidate.identity.package_id) ||
      candidate.identity.package_version.major == 0 ||
      candidate.identity.manifest_schema_version.major != 1 ||
      candidate.identity.manifest_schema_version.minor != 0 ||
      candidate.identity.manifest_schema_version.patch != 0 ||
      !known(candidate.identity.content.kind) ||
      !valid_key(candidate.identity.content.value) ||
      !valid_key(candidate.identity.publisher_id) ||
      !known(candidate.extension_class) || !known(candidate.category) ||
      ephemeral_identity(candidate.identity.package_id) ||
      ephemeral_identity(candidate.identity.content.value) ||
      ephemeral_identity(candidate.identity.publisher_id) ||
      candidate.requested_capabilities.empty()) {
    return false;
  }
  if (candidate.authority_claims.claims_trust ||
      candidate.authority_claims.claims_controller ||
      candidate.authority_claims.claims_profile) {
    return false;
  }
  if (candidate.extension_class == extensions::ExtensionClass::asset_pack &&
      candidate.contains_executable_content) {
    return false;
  }
  for (std::size_t index = 0; index < candidate.requested_capabilities.size();
       ++index) {
    if (!valid_key(candidate.requested_capabilities[index])) return false;
    for (std::size_t other = index + 1;
         other < candidate.requested_capabilities.size(); ++other) {
      if (candidate.requested_capabilities[index] ==
          candidate.requested_capabilities[other]) {
        return false;
      }
    }
  }
  return true;
}

bool PackagePolicy::decision_valid(
    const PackageCandidate& candidate,
    const PackageVerificationDecision& decision) const {
  if (decision.package_id != candidate.identity.package_id ||
      !known(decision.state) || !known(decision.provenance) ||
      !known(decision.assigned_trust) || !known(decision.controller) ||
      !valid_key(decision.signer_id) || !valid_key(decision.algorithm_id) ||
      !valid_key(decision.hardware_profile_id) ||
      !accepted_verification(decision.state)) {
    return false;
  }
  if (decision.state == PackageVerificationState::verified_trusted &&
      decision.assigned_trust != extensions::TrustClass::built_in &&
      decision.assigned_trust != extensions::TrustClass::signed_trusted) {
    return false;
  }
  if (decision.state == PackageVerificationState::verified_untrusted &&
      decision.assigned_trust != extensions::TrustClass::local_developer &&
      decision.assigned_trust != extensions::TrustClass::community_untrusted) {
    return false;
  }
  if (candidate.extension_class ==
          extensions::ExtensionClass::protected_safety_module &&
      (decision.state != PackageVerificationState::verified_trusted ||
       decision.assigned_trust != extensions::TrustClass::built_in ||
       decision.provenance != PackageTrustProvenance::built_in_image ||
       decision.controller != devices::ControllerIdentity::stm32_safety)) {
    return false;
  }
  return true;
}

bool PackagePolicy::compatible(const PackageCandidate& candidate) const {
  const auto sdk_version = sdk::supported_contract_range().minimum;
  return range_contains(candidate.sdk_contract,
                        {sdk_version.major, sdk_version.minor,
                         sdk_version.patch}) &&
         range_contains(candidate.core_contract,
                        extensions::supported_plugin_api_version());
}

bool PackagePolicy::registry_matches(const Record& record) const {
  const auto* extension = registry_.find(
      record.active.candidate.identity.package_id);
  if (extension == nullptr) return false;
  const auto& candidate = record.active.candidate;
  const auto& decision = record.active.decision;
  return extension->manifest.id == candidate.identity.package_id &&
         compare_version(extension->manifest.version,
                         candidate.identity.package_version) == 0 &&
         extension->manifest.extension_class == candidate.extension_class &&
         extension->manifest.category == candidate.category &&
         extension->assigned_trust == decision.assigned_trust &&
         extension->device_identity.controller.controller == decision.controller &&
         extension->device_identity.hardware_profile.profile_id ==
             decision.hardware_profile_id;
}

bool PackagePolicy::next_generation(Record& record) {
  if (record.installation_generation ==
      std::numeric_limits<std::uint64_t>::max()) {
    return false;
  }
  ++record.installation_generation;
  return record.installation_generation != 0;
}

PackageResult PackagePolicy::declare_available(
    const PackageCandidate& candidate) {
  if (max_packages_ == 0 || max_known_good_ == 0 || max_attempts_ == 0) {
    return PackageResult::rejected_invalid_policy;
  }
  if (find(candidate.identity.package_id) != nullptr) {
    return PackageResult::rejected_duplicate;
  }
  if (records_.size() >= max_packages_) {
    return PackageResult::rejected_invalid_policy;
  }
  if (candidate.authority_claims.claims_trust ||
      candidate.authority_claims.claims_controller ||
      candidate.authority_claims.claims_profile) {
    return PackageResult::rejected_self_authority;
  }
  if (!candidate_valid(candidate)) return PackageResult::rejected_identity;
  Record record;
  record.active.candidate = candidate;
  records_.push_back(std::move(record));
  return PackageResult::declared;
}

PackageResult PackagePolicy::stage(const std::string& package_id) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (record->lifecycle != PackageLifecycle::available) {
    return PackageResult::rejected_lifecycle;
  }
  record->lifecycle = PackageLifecycle::staged;
  return PackageResult::staged;
}

PackageResult PackagePolicy::verify(
    const std::string& package_id,
    const PackageVerificationDecision& decision) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (record->lifecycle != PackageLifecycle::staged) {
    return PackageResult::rejected_lifecycle;
  }
  if (!known(decision.state)) return PackageResult::rejected_unknown_domain;
  if (!decision_valid(record->active.candidate, decision)) {
    record->lifecycle = PackageLifecycle::quarantined;
    return PackageResult::rejected_verification;
  }
  record->active.decision = decision;
  record->active.verified = true;
  record->lifecycle = PackageLifecycle::verified;
  return PackageResult::verified;
}

PackageResult PackagePolicy::check_compatibility(
    const std::string& package_id) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (record->lifecycle != PackageLifecycle::verified) {
    return PackageResult::rejected_lifecycle;
  }
  if (!compatible(record->active.candidate)) {
    return PackageResult::rejected_incompatible;
  }
  record->active.compatible = true;
  record->lifecycle = PackageLifecycle::compatible;
  return PackageResult::compatible;
}

PackageResult PackagePolicy::mark_ready(const std::string& package_id) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (record->lifecycle != PackageLifecycle::compatible ||
      !record->active.verified || !record->active.compatible) {
    return PackageResult::rejected_lifecycle;
  }
  record->lifecycle = PackageLifecycle::ready;
  return PackageResult::ready;
}

PackageResult PackagePolicy::confirm_active(const std::string& package_id) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (record->lifecycle == PackageLifecycle::active &&
      registry_matches(*record)) {
    return PackageResult::activated;
  }
  if (record->lifecycle != PackageLifecycle::ready || !registry_matches(*record)) {
    return PackageResult::rejected_registry;
  }
  const auto* extension = registry_.find(package_id);
  if (extension->lifecycle != extensions::LifecycleState::active) {
    return PackageResult::rejected_host;
  }
  if (!next_generation(*record)) {
    return PackageResult::rejected_generation_exhausted;
  }
  record->lifecycle = PackageLifecycle::active;
  record->known_good.push_back(record->active);
  return PackageResult::activated;
}

PackageResult PackagePolicy::stage_update(
    const PackageCandidate& candidate,
    const std::uint64_t expected_generation,
    const DowngradePolicy downgrade_policy,
    const std::vector<std::string>& approved_capability_expansion) {
  auto* record = find_mutable(candidate.identity.package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (!known(downgrade_policy)) return PackageResult::rejected_unknown_domain;
  if (record->lifecycle != PackageLifecycle::active) {
    return PackageResult::rejected_lifecycle;
  }
  if (expected_generation != record->installation_generation) {
    return PackageResult::rejected_stale_generation;
  }
  if (record->update_attempts >= max_attempts_) {
    return PackageResult::rejected_retry_limit;
  }
  ++record->update_attempts;
  if (!candidate_valid(candidate) ||
      candidate.extension_class != record->active.candidate.extension_class ||
      candidate.category != record->active.candidate.category ||
      candidate.identity.publisher_id !=
          record->active.candidate.identity.publisher_id) {
    return PackageResult::rejected_identity;
  }
  const int version_order = compare_version(
      candidate.identity.package_version,
      record->active.candidate.identity.package_version);
  if (version_order == 0) {
    return candidate.identity.content.value !=
                   record->active.candidate.identity.content.value
               ? PackageResult::rejected_same_version_content
               : PackageResult::rejected_duplicate;
  }
  if (version_order < 0 && downgrade_policy != DowngradePolicy::allow_authorized) {
    return PackageResult::rejected_downgrade;
  }
  for (const auto& capability : candidate.requested_capabilities) {
    if (!contains(record->active.candidate.requested_capabilities, capability) &&
        !contains(approved_capability_expansion, capability)) {
      return PackageResult::rejected_capability_expansion;
    }
  }
  record->pending = VersionRecord{candidate, {}, false, false};
  record->approved_expansion = approved_capability_expansion;
  record->lifecycle = PackageLifecycle::update_pending;
  return PackageResult::update_staged;
}

PackageResult PackagePolicy::verify_update(
    const PackageVerificationDecision& decision) {
  auto* record = find_mutable(decision.package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (record->lifecycle != PackageLifecycle::update_pending ||
      !record->pending.has_value()) {
    return PackageResult::rejected_lifecycle;
  }
  if (!known(decision.state)) return PackageResult::rejected_unknown_domain;
  if (!decision_valid(record->pending->candidate, decision) ||
      decision.assigned_trust != record->active.decision.assigned_trust ||
      decision.provenance != record->active.decision.provenance ||
      decision.controller != record->active.decision.controller ||
      decision.hardware_profile_id !=
          record->active.decision.hardware_profile_id ||
      decision.signer_id != record->active.decision.signer_id) {
    record->pending.reset();
    record->lifecycle = PackageLifecycle::active;
    return PackageResult::rejected_verification;
  }
  record->pending->decision = decision;
  record->pending->verified = true;
  return PackageResult::verified;
}

PackageResult PackagePolicy::check_update_compatibility(
    const std::string& package_id) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (record->lifecycle != PackageLifecycle::update_pending ||
      !record->pending.has_value() || !record->pending->verified) {
    return PackageResult::rejected_lifecycle;
  }
  if (!compatible(record->pending->candidate)) {
    record->pending.reset();
    record->lifecycle = PackageLifecycle::active;
    return PackageResult::rejected_incompatible;
  }
  record->pending->compatible = true;
  return PackageResult::compatible;
}

PackageResult PackagePolicy::commit_update(const std::string& package_id,
                                           ExtensionHost& host) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (record->lifecycle != PackageLifecycle::update_pending ||
      !record->pending.has_value() || !record->pending->verified ||
      !record->pending->compatible) {
    return PackageResult::rejected_lifecycle;
  }
  if (record->installation_generation ==
      std::numeric_limits<std::uint64_t>::max()) {
    return PackageResult::rejected_generation_exhausted;
  }
  const auto* registry_record = registry_.find(package_id);
  if (registry_record == nullptr) return PackageResult::rejected_registry;
  for (const auto& capability : record->pending->candidate.requested_capabilities) {
    if (!contains(registry_record->validated_capabilities, capability)) {
      return PackageResult::rejected_capability_expansion;
    }
  }
  const auto previous = record->active;
  const auto previous_generation = registry_record->authorization_generation;
  if (host.suspend_extension(package_id) != ExtensionHostResult::suspended) {
    return PackageResult::rejected_host;
  }
  registry_record = registry_.find(package_id);
  if (registry_.replace_package_version(
          package_id, record->pending->candidate.identity.package_version,
          registry_record->authorization_generation) !=
      extensions::RegistryResult::package_version_replaced) {
    record->host_transition_authorized = true;
    const auto restored = host.activate_extension(
        package_id, previous.candidate.requested_capabilities);
    record->host_transition_authorized = false;
    record->pending.reset();
    record->lifecycle = restored == ExtensionHostResult::activated
                            ? PackageLifecycle::active
                            : PackageLifecycle::quarantined;
    return restored == ExtensionHostResult::activated
               ? PackageResult::rejected_registry
               : PackageResult::rejected_atomic_restore;
  }
  record->host_transition_authorized = true;
  const auto activation = host.activate_extension(
      package_id, record->pending->candidate.requested_capabilities);
  record->host_transition_authorized = false;
  if (activation != ExtensionHostResult::activated) {
    if (host.lifecycle(package_id) == sdk::ExtensionLifecycle::failed) {
      host.recover_extension(package_id);
    }
    const auto* current = registry_.find(package_id);
    const auto version_restored = current != nullptr &&
        registry_.replace_package_version(
            package_id, previous.candidate.identity.package_version,
            current->authorization_generation) ==
            extensions::RegistryResult::package_version_replaced;
    record->host_transition_authorized = true;
    const auto authority_restored = version_restored &&
        host.activate_extension(package_id,
                                previous.candidate.requested_capabilities) ==
            ExtensionHostResult::activated;
    record->host_transition_authorized = false;
    record->pending.reset();
    record->lifecycle = authority_restored ? PackageLifecycle::active
                                           : PackageLifecycle::quarantined;
    return authority_restored ? PackageResult::rejected_host
                              : PackageResult::rejected_atomic_restore;
  }
  (void)previous_generation;
  record->known_good.push_back(previous);
  if (record->known_good.size() > max_known_good_) {
    record->known_good.erase(record->known_good.begin());
  }
  record->active = *record->pending;
  record->pending.reset();
  if (!next_generation(*record)) {
    return PackageResult::rejected_generation_exhausted;
  }
  record->lifecycle = PackageLifecycle::active;
  record->update_attempts = 0;
  return PackageResult::updated;
}

PackageResult PackagePolicy::rollback(
    const std::string& package_id,
    const std::string& target_content_identity,
    const std::uint64_t expected_generation, ExtensionHost& host) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (record->lifecycle != PackageLifecycle::active) {
    return PackageResult::rejected_lifecycle;
  }
  if (record->installation_generation != expected_generation) {
    return PackageResult::rejected_stale_generation;
  }
  if (record->rollback_attempts >= max_attempts_) {
    return PackageResult::rejected_retry_limit;
  }
  if (record->installation_generation ==
      std::numeric_limits<std::uint64_t>::max()) {
    return PackageResult::rejected_generation_exhausted;
  }
  ++record->rollback_attempts;
  const auto target = std::find_if(
      record->known_good.rbegin(), record->known_good.rend(),
      [&target_content_identity](const VersionRecord& version) {
        return version.candidate.identity.content.value ==
                   target_content_identity &&
               version.verified && version.compatible;
      });
  if (target == record->known_good.rend()) {
    return PackageResult::rejected_missing_rollback;
  }
  record->lifecycle = PackageLifecycle::rollback_pending;
  const auto previous = record->active;
  if (host.suspend_extension(package_id) != ExtensionHostResult::suspended) {
    record->lifecycle = PackageLifecycle::active;
    return PackageResult::rejected_host;
  }
  const auto* current = registry_.find(package_id);
  if (registry_.replace_package_version(
          package_id, target->candidate.identity.package_version,
          current->authorization_generation) !=
      extensions::RegistryResult::package_version_replaced) {
    record->host_transition_authorized = true;
    const auto restored = host.activate_extension(
        package_id, previous.candidate.requested_capabilities);
    record->host_transition_authorized = false;
    record->lifecycle = restored == ExtensionHostResult::activated
                            ? PackageLifecycle::active
                            : PackageLifecycle::quarantined;
    return restored == ExtensionHostResult::activated
               ? PackageResult::rejected_registry
               : PackageResult::rejected_atomic_restore;
  }
  record->host_transition_authorized = true;
  const auto activation =
      host.activate_extension(package_id, target->candidate.requested_capabilities);
  record->host_transition_authorized = false;
  if (activation != ExtensionHostResult::activated) {
    if (host.lifecycle(package_id) == sdk::ExtensionLifecycle::failed) {
      host.recover_extension(package_id);
    }
    const auto* failed = registry_.find(package_id);
    bool version_restored = false;
    if (failed != nullptr) {
      version_restored = registry_.replace_package_version(
          package_id, previous.candidate.identity.package_version,
          failed->authorization_generation) ==
          extensions::RegistryResult::package_version_replaced;
      record->host_transition_authorized = true;
      const auto authority_restored = version_restored &&
          host.activate_extension(package_id,
                                  previous.candidate.requested_capabilities) ==
              ExtensionHostResult::activated;
      record->host_transition_authorized = false;
      record->lifecycle = authority_restored ? PackageLifecycle::active
                                             : PackageLifecycle::quarantined;
      return authority_restored ? PackageResult::rejected_host
                                : PackageResult::rejected_atomic_restore;
    }
    record->lifecycle = PackageLifecycle::quarantined;
    return PackageResult::rejected_atomic_restore;
  }
  record->active = *target;
  if (!next_generation(*record)) {
    return PackageResult::rejected_generation_exhausted;
  }
  record->known_good.push_back(previous);
  if (record->known_good.size() > max_known_good_) {
    record->known_good.erase(record->known_good.begin());
  }
  record->lifecycle = PackageLifecycle::active;
  record->rollback_attempts = 0;
  return PackageResult::rolled_back;
}

PackageResult PackagePolicy::quarantine(const std::string& package_id,
                                        ExtensionHost& host) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (host.quarantine_extension(package_id) !=
      ExtensionHostResult::quarantined) {
    return PackageResult::rejected_host;
  }
  record->lifecycle = PackageLifecycle::quarantined;
  return PackageResult::quarantined;
}

PackageResult PackagePolicy::disable(const std::string& package_id,
                                     ExtensionHost& host) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (host.quarantine_extension(package_id) !=
      ExtensionHostResult::quarantined) {
    return PackageResult::rejected_host;
  }
  record->lifecycle = PackageLifecycle::disabled;
  return PackageResult::disabled;
}

PackageResult PackagePolicy::remove(const std::string& package_id,
                                    ExtensionHost& host) {
  auto* record = find_mutable(package_id);
  if (record == nullptr) return PackageResult::rejected_not_found;
  if (host.remove_extension(package_id) != ExtensionHostResult::removed) {
    return PackageResult::rejected_host;
  }
  record->lifecycle = PackageLifecycle::removed;
  return PackageResult::removed;
}

bool PackagePolicy::allows_activation(const std::string& package_id) const {
  const auto* record = find(package_id);
  return record != nullptr &&
         (record->lifecycle == PackageLifecycle::ready ||
          record->lifecycle == PackageLifecycle::active ||
          record->host_transition_authorized) &&
         record->active.verified && record->active.compatible;
}

bool PackagePolicy::runtime_ready(
    const std::vector<std::string>& required_packages) const {
  for (const auto& package_id : required_packages) {
    const auto* record = find(package_id);
    if (record == nullptr || record->lifecycle != PackageLifecycle::active ||
        !registry_matches(*record)) {
      return false;
    }
  }
  return true;
}

std::optional<PackageSnapshot> PackagePolicy::snapshot(
    const std::string& package_id) const {
  const auto* record = find(package_id);
  if (record == nullptr) return std::nullopt;
  return PackageSnapshot{record->active.candidate.identity,
                         record->lifecycle,
                         record->active.decision.state,
                         record->active.decision.assigned_trust,
                         record->active.decision.provenance,
                         record->active.decision.controller,
                         record->active.decision.hardware_profile_id,
                         record->installation_generation};
}

SecretPolicyResult SecretBoundaryPolicy::validate(
    const SecretSurface surface, const std::string& key,
    const std::string& value) const {
  switch (surface) {
    case SecretSurface::manifest:
    case SecretSurface::hardware_profile:
    case SecretSurface::runtime_checkpoint:
    case SecretSurface::log:
    case SecretSurface::event:
    case SecretSurface::package_metadata:
    case SecretSurface::plugin_configuration:
      if (secret_token(key)) {
        return SecretPolicyResult::rejected_secret_namespace;
      }
      if (secret_token(value)) return SecretPolicyResult::rejected_secret_value;
      return SecretPolicyResult::accepted;
    case SecretSurface::provider_credential_handle:
      return key == "credential-handle" && valid_key(value)
                 ? SecretPolicyResult::accepted
                 : SecretPolicyResult::rejected_secret_value;
  }
  return SecretPolicyResult::rejected_unknown_surface;
}

SecretPolicyResult SecretBoundaryPolicy::authorize_handle(
    const std::string& requester_package_id,
    const OpaqueCredentialHandle& handle) const {
  if (!valid_key(requester_package_id) || !valid_key(handle.handle_id) ||
      !valid_key(handle.owner_package_id) ||
      !valid_key(handle.provider_domain)) {
    return SecretPolicyResult::rejected_secret_value;
  }
  return requester_package_id == handle.owner_package_id
             ? SecretPolicyResult::accepted
             : SecretPolicyResult::rejected_cross_package_access;
}

WatchdogDecision RuntimeWatchdogPolicy::observe(
    const WatchdogObservation& observation) {
  if (!known(observation.health)) {
    return {WatchdogAction::fail_safe_stop_and_revoke,
            WatchdogReason::unknown_domain, observation.generation};
  }
  switch (observation.domain) {
    case WatchdogObservationDomain::host_runtime:
      if (observation.generation == 0 ||
          observation.generation <= last_runtime_generation_) {
        return {WatchdogAction::fail_safe_stop_and_revoke,
                WatchdogReason::runtime_generation_stale,
                observation.generation};
      }
      last_runtime_generation_ = observation.generation;
      break;
    case WatchdogObservationDomain::controller_session:
      if (observation.health != WatchdogHealth::healthy) {
        return {WatchdogAction::fail_safe_stop_and_revoke,
                WatchdogReason::controller_liveness_lost,
                observation.generation};
      }
      break;
    case WatchdogObservationDomain::required_subsystem:
      if (observation.required &&
          observation.health != WatchdogHealth::healthy) {
        return {WatchdogAction::fail_safe_stop_and_revoke,
                WatchdogReason::required_safety_lost,
                observation.generation};
      }
      break;
    case WatchdogObservationDomain::extension:
      if (observation.health == WatchdogHealth::failed) {
        if (extension_failures_ < max_repeated_failures_) {
          ++extension_failures_;
        }
        return {extension_failures_ >= max_repeated_failures_
                    ? WatchdogAction::quarantine_extension
                    : WatchdogAction::degrade_runtime,
                WatchdogReason::repeated_extension_failure,
                observation.generation};
      }
      break;
    case WatchdogObservationDomain::provider:
      if (observation.health != WatchdogHealth::healthy) {
        if (provider_failures_ < max_repeated_failures_) {
          ++provider_failures_;
        }
        return {WatchdogAction::degrade_runtime,
                WatchdogReason::repeated_provider_failure,
                observation.generation};
      }
      break;
    case WatchdogObservationDomain::package_update:
      if (observation.health == WatchdogHealth::stuck) {
        return {WatchdogAction::abort_update, WatchdogReason::update_stuck,
                observation.generation};
      }
      break;
    case WatchdogObservationDomain::package_rollback:
      if (observation.health == WatchdogHealth::stuck) {
        return {WatchdogAction::abort_rollback, WatchdogReason::rollback_stuck,
                observation.generation};
      }
      break;
    default:
      return {WatchdogAction::fail_safe_stop_and_revoke,
              WatchdogReason::unknown_domain, observation.generation};
  }
  return {WatchdogAction::none, WatchdogReason::none,
          observation.generation};
}

}  // namespace zie::core
