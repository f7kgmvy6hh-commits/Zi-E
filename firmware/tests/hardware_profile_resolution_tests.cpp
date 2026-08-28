#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "core/AuthoritativeRobotCore.hpp"
#include "core/ExtensionHost.hpp"
#include "zie/core/HardwareProfile.hpp"

namespace {

using namespace zie;

extensions::ExtensionCandidate hardware_candidate(
    const std::string& id, const std::string& capability,
    const extensions::ExtensionCategory category, const std::string& profile_id,
    const extensions::ExtensionClass extension_class =
        extensions::ExtensionClass::embedded_module) {
  extensions::ExtensionCandidate value;
  value.manifest.id = id;
  value.manifest.name = "Profile device";
  value.manifest.version = {1, 0, 0};
  value.manifest.plugin_api_version = {1, 0, 0};
  value.manifest.manifest_schema_version = {1, 0, 0};
  value.manifest.category = category;
  value.manifest.extension_class = extension_class;
  value.manifest.controller_target =
      extension_class == extensions::ExtensionClass::host_plugin
          ? extensions::ControllerTarget::host
          : extension_class ==
                    extensions::ExtensionClass::protected_safety_module
                ? extensions::ControllerTarget::stm32
                : extensions::ControllerTarget::esp32_s3;
  value.manifest.entrypoint = "profile_device_entry";
  value.manifest.declared_capabilities = {capability};
  value.device_identity.package.extension_id = id;
  value.device_identity.physical.state =
      devices::PhysicalIdentityState::provisional_local;
  value.device_identity.physical.manufacturer = "zie";
  value.device_identity.physical.model = "profile-device";
  value.device_identity.physical.provisional_local_id = id + ".physical";
  value.device_identity.physical.provenance = {
      devices::IdentitySource::registry_generated,
      devices::IdentityTrust::provisional};
  value.device_identity.logical.instance_id = id + ".logical";
  value.device_identity.controller.controller =
      extension_class == extensions::ExtensionClass::host_plugin
          ? devices::ControllerIdentity::host
          : extension_class ==
                    extensions::ExtensionClass::protected_safety_module
                ? devices::ControllerIdentity::stm32_safety
                : devices::ControllerIdentity::esp32_s3;
  value.device_identity.hardware_profile.profile_id = profile_id;
  return value;
}

extensions::RegistryAssignment assignment_for(
    const extensions::ExtensionCandidate& candidate,
    const extensions::TrustClass trust =
        extensions::TrustClass::local_developer,
    std::vector<extensions::SemanticDeviceAttribute> attributes = {}) {
  return {candidate.manifest.id, trust,
          candidate.device_identity.controller.controller,
          candidate.device_identity.hardware_profile.profile_id,
          std::move(attributes)};
}

void register_validated_inactive(
    extensions::ExtensionRegistry& registry,
    const extensions::ExtensionCandidate& candidate,
    const extensions::TrustClass trust =
        extensions::TrustClass::local_developer,
    std::vector<extensions::SemanticDeviceAttribute> attributes = {}) {
  assert(registry.register_extension(
             candidate,
             assignment_for(candidate, trust, std::move(attributes))) ==
         extensions::RegistryResult::accepted);
  assert(registry.transition(candidate.manifest.id,
                             extensions::LifecycleState::validated) ==
         extensions::RegistryResult::transitioned);
  assert(registry.validate_capabilities(
             candidate.manifest.id,
             candidate.manifest.declared_capabilities) ==
         extensions::RegistryResult::capabilities_validated);
  assert(registry.transition(candidate.manifest.id,
                             extensions::LifecycleState::inactive) ==
         extensions::RegistryResult::transitioned);
}

void activate_registered(extensions::ExtensionRegistry& registry,
                         const extensions::ExtensionCandidate& candidate) {
  assert(registry.transition(candidate.manifest.id,
                             extensions::LifecycleState::activating) ==
         extensions::RegistryResult::transitioned);
  assert(registry.activate_capabilities(
             candidate.manifest.id,
             candidate.manifest.declared_capabilities) ==
         extensions::RegistryResult::capabilities_activated);
}

void register_active(extensions::ExtensionRegistry& registry,
                     const extensions::ExtensionCandidate& candidate,
                     const extensions::TrustClass trust =
                         extensions::TrustClass::local_developer,
                     std::vector<extensions::SemanticDeviceAttribute> attributes =
                         {}) {
  register_validated_inactive(registry, candidate, trust,
                              std::move(attributes));
  activate_registered(registry, candidate);
}

core::HardwareProfileEntry entry(
    const core::HardwareDomain domain, std::string slot,
    std::string capability, const extensions::ExtensionCategory category,
    const core::ProfileRequirement requirement =
        core::ProfileRequirement::required,
    const core::ProfileCardinality cardinality =
        core::ProfileCardinality::exactly_one,
    const core::ProfileOwnership ownership =
        core::ProfileOwnership::registry_managed) {
  return {domain, std::move(slot), std::move(capability), category, requirement,
          cardinality, ownership, {}};
}

core::HardwareProfileDefinition profile(
    std::string id, std::vector<core::HardwareProfileEntry> entries,
    const std::uint64_t revision = 1) {
  return {std::move(id), {1, 0, 0}, revision,
          core::RobotPlatformClass::zie_desktop_companion,
          std::move(entries)};
}

core::HardwareProfileAuthority profile_authority(const std::string& id) {
  return {id, "zie.profile-authority", core::ProfileSource::operator_approved};
}

void declare_and_validate(core::HardwareProfileManager& manager,
                          const core::HardwareProfileDefinition& definition) {
  assert(manager.declare_profile(definition,
                                 profile_authority(definition.profile_id)) ==
         core::HardwareProfileResult::declared);
  assert(manager.validate_profile(definition.profile_id) ==
         core::HardwareProfileResult::validated);
}

class ProfileSdkMock final : public sdk::Extension {
 public:
  bool initialize(std::shared_ptr<const sdk::ExtensionContext> context) override {
    initialized = std::move(context);
    return true;
  }
  bool activate(std::shared_ptr<const sdk::ExtensionContext> context) override {
    active = std::move(context);
    return true;
  }
  void suspended() override {}

  std::shared_ptr<const sdk::ExtensionContext> initialized;
  std::shared_ptr<const sdk::ExtensionContext> active;
};

}  // namespace

void run_hardware_profile_resolution_tests() {
  using namespace zie;

  {
    extensions::ExtensionRegistry registry;
    const auto display = hardware_candidate(
        "zie.profile.display", "hardware.display",
        extensions::ExtensionCategory::hardware_display, "zie.profile.minimal");
    const auto range = hardware_candidate(
        "zie.profile.range", "hardware.range.front",
        extensions::ExtensionCategory::hardware_range, "zie.profile.minimal");
    register_active(
        registry, display, extensions::TrustClass::local_developer,
        {{extensions::SemanticAttributeDomain::placement, "front"}});
    register_active(registry, range);
    core::HardwareProfileManager manager(registry, 4, 16, 4);
    auto display_entry =
        entry(core::HardwareDomain::display, "display.primary",
              "hardware.display",
              extensions::ExtensionCategory::hardware_display);
    display_entry.constraints.push_back(
        {core::SemanticConstraintDomain::placement, "front"});
    const auto minimal = profile(
        "zie.profile.minimal",
        {std::move(display_entry),
         entry(core::HardwareDomain::range, "range.front",
               "hardware.range.front",
               extensions::ExtensionCategory::hardware_range)});
    declare_and_validate(manager, minimal);
    assert(manager.resolve_profile(minimal.profile_id) ==
           core::HardwareProfileResult::resolved);
    const auto resolved = manager.resolution(minimal.profile_id);
    assert(resolved.has_value());
    assert(resolved->ready);
    assert(resolved->entries.size() == 2);
    assert(manager.activate_profile(minimal.profile_id) ==
           core::HardwareProfileResult::activated);
    assert(manager.active_profile()->identity.profile_id == minimal.profile_id);
  }

  {
    extensions::ExtensionRegistry registry;
    const auto display = hardware_candidate(
        "zie.profile.full-display", "hardware.display",
        extensions::ExtensionCategory::hardware_display, "zie.profile.full");
    register_active(registry, display);
    core::HardwareProfileManager manager(registry, 2, 16, 4);
    const auto full = profile(
        "zie.profile.full",
        {entry(core::HardwareDomain::display, "display.primary",
               "hardware.display",
               extensions::ExtensionCategory::hardware_display),
         entry(core::HardwareDomain::camera, "camera.primary",
               "hardware.camera",
               extensions::ExtensionCategory::hardware_camera,
               core::ProfileRequirement::optional,
               core::ProfileCardinality::zero_or_one),
         entry(core::HardwareDomain::audio_output, "audio.output",
               "hardware.audio-output",
               extensions::ExtensionCategory::hardware_audio_output,
               core::ProfileRequirement::optional,
               core::ProfileCardinality::zero_or_one)});
    declare_and_validate(manager, full);
    assert(manager.resolve_profile(full.profile_id) ==
           core::HardwareProfileResult::resolved);
    const auto resolved = *manager.resolution(full.profile_id);
    assert(resolved.ready);
    assert(resolved.entries[1].status == core::ProfileEntryStatus::optional_missing);
    assert(resolved.entries[2].status == core::ProfileEntryStatus::optional_missing);
  }

  {
    extensions::ExtensionRegistry registry;
    core::HardwareProfileManager manager(registry, 8, 16, 4);
    auto duplicate = profile(
        "zie.profile.duplicate",
        {entry(core::HardwareDomain::display, "shared.slot", "hardware.display",
               extensions::ExtensionCategory::hardware_display),
         entry(core::HardwareDomain::camera, "shared.slot", "hardware.camera",
               extensions::ExtensionCategory::hardware_camera)});
    assert(manager.declare_profile(duplicate,
                                   profile_authority(duplicate.profile_id)) ==
           core::HardwareProfileResult::declared);
    assert(manager.validate_profile(duplicate.profile_id) ==
           core::HardwareProfileResult::rejected_validation);
    const auto duplicate_issues =
        manager.validation_issues(duplicate.profile_id);
    assert(std::find(duplicate_issues.begin(), duplicate_issues.end(),
                     core::HardwareProfileIssue::duplicate_slot) !=
           duplicate_issues.end());

    auto unknown = profile(
        "zie.profile.unknown",
        {entry(static_cast<core::HardwareDomain>(999), "unknown.slot",
               "hardware.display",
               static_cast<extensions::ExtensionCategory>(999),
               core::ProfileRequirement::required,
               static_cast<core::ProfileCardinality>(999))});
    unknown.schema_version = {2, 0, 0};
    assert(manager.declare_profile(unknown,
                                   profile_authority(unknown.profile_id)) ==
           core::HardwareProfileResult::declared);
    assert(manager.validate_profile(unknown.profile_id) ==
           core::HardwareProfileResult::rejected_validation);
    const auto unknown_issues = manager.validation_issues(unknown.profile_id);
    assert(std::find(unknown_issues.begin(), unknown_issues.end(),
                     core::HardwareProfileIssue::unknown_schema) !=
           unknown_issues.end());
    assert(std::find(unknown_issues.begin(), unknown_issues.end(),
                     core::HardwareProfileIssue::unknown_domain) !=
           unknown_issues.end());
    assert(std::find(unknown_issues.begin(), unknown_issues.end(),
                     core::HardwareProfileIssue::unknown_cardinality) !=
           unknown_issues.end());
  }

  {
    extensions::ExtensionRegistry registry;
    core::HardwareProfileManager manager(registry, 2, 8, 2);
    const auto missing = profile(
        "zie.profile.missing",
        {entry(core::HardwareDomain::battery, "battery.primary",
               "hardware.battery",
               extensions::ExtensionCategory::hardware_battery,
               core::ProfileRequirement::required,
               core::ProfileCardinality::exactly_one,
               core::ProfileOwnership::protected_safety_core)});
    declare_and_validate(manager, missing);
    assert(manager.resolve_profile(missing.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(!manager.resolution(missing.profile_id)->ready);
    assert(manager.resolution(missing.profile_id)->entries[0].status ==
           core::ProfileEntryStatus::required_missing);
    assert(manager.activate_profile(missing.profile_id) ==
           core::HardwareProfileResult::rejected_unresolved);
  }

  {
    extensions::ExtensionRegistry registry;
    const auto first = hardware_candidate(
        "zie.profile.display-a", "hardware.display.variant-a",
        extensions::ExtensionCategory::hardware_display, "zie.profile.ambiguous");
    const auto second = hardware_candidate(
        "zie.profile.display-b", "hardware.display.variant-b",
        extensions::ExtensionCategory::hardware_display, "zie.profile.ambiguous");
    register_active(registry, first);
    register_active(registry, second);
    core::HardwareProfileManager manager(registry, 2, 8, 4);
    const auto ambiguous = profile(
        "zie.profile.ambiguous",
        {entry(core::HardwareDomain::display, "display.primary",
               "hardware.display",
               extensions::ExtensionCategory::hardware_display)});
    declare_and_validate(manager, ambiguous);
    assert(manager.resolve_profile(ambiguous.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(manager.resolution(ambiguous.profile_id)->entries[0].status ==
           core::ProfileEntryStatus::ambiguous);
    assert(!manager.resolution(ambiguous.profile_id)->ready);
  }

  {
    extensions::ExtensionRegistry registry;
    const auto unsafe = hardware_candidate(
        "zie.profile.unsafe-variant", "hardware.range.raw_gpio",
        extensions::ExtensionCategory::hardware_range,
        "zie.profile.safe-range");
    register_active(registry, unsafe);
    core::HardwareProfileManager manager(registry, 2, 8, 2);
    const auto safe = profile(
        "zie.profile.safe-range",
        {entry(core::HardwareDomain::range, "range.primary", "hardware.range",
               extensions::ExtensionCategory::hardware_range)});
    declare_and_validate(manager, safe);
    assert(manager.resolve_profile(safe.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(!manager.resolution(safe.profile_id)->ready);
    assert(manager.resolution(safe.profile_id)->entries[0].status ==
           core::ProfileEntryStatus::unsafe_capability);
  }

  {
    extensions::ExtensionRegistry registry;
    const auto display = hardware_candidate(
        "zie.profile.constraint-display", "hardware.display",
        extensions::ExtensionCategory::hardware_display,
        "zie.profile.constraint");
    register_active(
        registry, display, extensions::TrustClass::local_developer,
        {{extensions::SemanticAttributeDomain::placement, "rear"}});
    core::HardwareProfileManager manager(registry, 2, 8, 2);
    auto constrained_entry =
        entry(core::HardwareDomain::display, "display.primary",
              "hardware.display",
              extensions::ExtensionCategory::hardware_display);
    constrained_entry.constraints.push_back(
        {core::SemanticConstraintDomain::placement, "front"});
    const auto constrained = profile("zie.profile.constraint",
                                     {std::move(constrained_entry)});
    declare_and_validate(manager, constrained);
    assert(manager.resolve_profile(constrained.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(!manager.resolution(constrained.profile_id)->ready);
    assert(manager.resolution(constrained.profile_id)->entries[0].status ==
           core::ProfileEntryStatus::incompatible_constraints);
  }

  {
    extensions::ExtensionRegistry registry;
    const auto wrong = hardware_candidate(
        "zie.profile.wrong-binding", "hardware.camera",
        extensions::ExtensionCategory::hardware_camera, "zie.profile.other");
    register_active(registry, wrong);
    core::HardwareProfileManager manager(registry, 3, 8, 2);
    const auto expected = profile(
        "zie.profile.expected",
        {entry(core::HardwareDomain::camera, "camera.primary",
               "hardware.camera",
               extensions::ExtensionCategory::hardware_camera)});
    declare_and_validate(manager, expected);
    assert(manager.resolve_profile(expected.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(manager.resolution(expected.profile_id)->entries[0].status ==
           core::ProfileEntryStatus::wrong_profile);

    const auto inactive = hardware_candidate(
        "zie.profile.inactive", "hardware.range",
        extensions::ExtensionCategory::hardware_range, "zie.profile.inactive");
    register_active(registry, inactive);
    assert(registry.transition(inactive.manifest.id,
                               extensions::LifecycleState::quarantined,
                               extensions::FailureClass::security) ==
           extensions::RegistryResult::transitioned);
    const auto inactive_profile = profile(
        "zie.profile.inactive",
        {entry(core::HardwareDomain::range, "range.primary", "hardware.range",
               extensions::ExtensionCategory::hardware_range)});
    declare_and_validate(manager, inactive_profile);
    assert(manager.resolve_profile(inactive_profile.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(manager.resolution(inactive_profile.profile_id)->entries[0].status ==
           core::ProfileEntryStatus::inactive);

    const auto optional_inactive_profile = profile(
        "zie.profile.optional-inactive",
        {entry(core::HardwareDomain::range, "range.optional", "hardware.range",
               extensions::ExtensionCategory::hardware_range,
               core::ProfileRequirement::optional,
               core::ProfileCardinality::zero_or_one)});
    assert(manager.declare_profile(
               optional_inactive_profile,
               profile_authority(optional_inactive_profile.profile_id)) ==
           core::HardwareProfileResult::declared);
    assert(manager.validate_profile(optional_inactive_profile.profile_id) ==
           core::HardwareProfileResult::validated);
    assert(manager.resolve_profile(optional_inactive_profile.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(manager.resolution(optional_inactive_profile.profile_id)->ready);
  }

  {
    extensions::ExtensionRegistry registry;
    auto impersonator = hardware_candidate(
        "zie.profile.impersonator", "hardware.camera",
        extensions::ExtensionCategory::hardware_camera, "zie.profile.identity");
    impersonator.device_identity.package.extension_id = "zie.profile.victim";
    assert(registry.register_extension(impersonator,
                                       assignment_for(impersonator)) ==
           extensions::RegistryResult::rejected_package_mismatch);

    core::HardwareProfileManager manager(registry, 2, 8, 2);
    const auto identity_profile = profile("zie.profile.authoritative", {});
    auto wrong_authority = profile_authority("zie.profile.other");
    assert(manager.declare_profile(identity_profile, wrong_authority) ==
           core::HardwareProfileResult::rejected_identity);
  }

  {
    extensions::ExtensionRegistry registry;
    const auto old_display = hardware_candidate(
        "zie.profile.old-display", "hardware.display",
        extensions::ExtensionCategory::hardware_display, "zie.profile.old");
    register_active(registry, old_display);
    core::HardwareProfileManager manager(registry, 6, 8, 2);
    const auto old_profile = profile(
        "zie.profile.old",
        {entry(core::HardwareDomain::display, "display.primary",
               "hardware.display",
               extensions::ExtensionCategory::hardware_display)});
    declare_and_validate(manager, old_profile);
    assert(manager.resolve_profile(old_profile.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(manager.activate_profile(old_profile.profile_id) ==
           core::HardwareProfileResult::activated);

    const auto replacement = profile(
        "zie.profile.replacement",
        {entry(core::HardwareDomain::camera, "camera.primary",
               "hardware.camera",
               extensions::ExtensionCategory::hardware_camera)});
    declare_and_validate(manager, replacement);
    assert(manager.resolve_profile(replacement.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(manager.activate_profile(replacement.profile_id) ==
           core::HardwareProfileResult::rejected_unresolved);
    assert(manager.active_profile()->identity.profile_id == old_profile.profile_id);
  }

  {
    extensions::ExtensionRegistry registry;
    const auto first = hardware_candidate(
        "zie.profile.rebind-a", "hardware.range",
        extensions::ExtensionCategory::hardware_range, "zie.profile.rebind");
    const auto second = hardware_candidate(
        "zie.profile.rebind-b", "hardware.range",
        extensions::ExtensionCategory::hardware_range, "zie.profile.rebind");
    register_active(registry, first);
    register_validated_inactive(registry, second);
    core::HardwareProfileManager manager(registry, 2, 8, 4);
    const auto rebind = profile(
        "zie.profile.rebind",
        {entry(core::HardwareDomain::range, "range.primary", "hardware.range",
               extensions::ExtensionCategory::hardware_range)});
    declare_and_validate(manager, rebind);
    assert(manager.resolve_profile(rebind.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(manager.activate_profile(rebind.profile_id) ==
           core::HardwareProfileResult::activated);
    const auto original_generation =
        manager.active_profile()->identity.generation;
    assert(registry.transition(first.manifest.id,
                               extensions::LifecycleState::inactive) ==
           extensions::RegistryResult::transitioned);
    activate_registered(registry, second);
    assert(!manager.active_profile().has_value());
    assert(manager.state(rebind.profile_id) ==
           core::HardwareProfileState::invalidated);
    assert(manager.resolution(rebind.profile_id)->entries[0].status ==
           core::ProfileEntryStatus::revoked);
    assert(manager.resolution(rebind.profile_id)->identity.generation ==
           original_generation);
    assert(manager.resolve_profile(rebind.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(manager.resolution(rebind.profile_id)
               ->entries[0]
               .devices[0]
               .package_id == second.manifest.id);
    assert(manager.activate_profile(rebind.profile_id) ==
           core::HardwareProfileResult::activated);
    assert(manager.active_profile()->identity.generation != original_generation);
  }

  {
    extensions::ExtensionRegistry registry;
    const auto unsafe_plugin = hardware_candidate(
        "zie.profile.unsafe-drive", "hardware.drive",
        extensions::ExtensionCategory::hardware_drive, "zie.profile.safety",
        extensions::ExtensionClass::host_plugin);
    register_active(registry, unsafe_plugin);
    core::HardwareProfileManager manager(registry, 6, 8, 2);
    const auto safety = profile(
        "zie.profile.safety",
        {entry(core::HardwareDomain::drive, "drive.primary", "hardware.drive",
               extensions::ExtensionCategory::hardware_drive,
               core::ProfileRequirement::required,
               core::ProfileCardinality::exactly_one,
               core::ProfileOwnership::protected_safety_core)});
    declare_and_validate(manager, safety);
    assert(manager.resolve_profile(safety.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(manager.resolution(safety.profile_id)->entries[0].status ==
           core::ProfileEntryStatus::protected_ownership);

    const auto unprotected_drive = profile(
        "zie.profile.unprotected-drive",
        {entry(core::HardwareDomain::drive, "drive.primary", "hardware.drive",
               extensions::ExtensionCategory::hardware_drive)});
    assert(manager.declare_profile(
               unprotected_drive,
               profile_authority(unprotected_drive.profile_id)) ==
           core::HardwareProfileResult::declared);
    assert(manager.validate_profile(unprotected_drive.profile_id) ==
           core::HardwareProfileResult::rejected_validation);

    auto raw = profile(
        "zie.profile.raw",
        {entry(core::HardwareDomain::display, "gpio.pin", "gpio.pin",
               extensions::ExtensionCategory::hardware_display,
               core::ProfileRequirement::required,
               core::ProfileCardinality::exactly_one,
               core::ProfileOwnership::protected_safety_core)});
    assert(manager.declare_profile(raw, profile_authority(raw.profile_id)) ==
           core::HardwareProfileResult::declared);
    assert(manager.validate_profile(raw.profile_id) ==
           core::HardwareProfileResult::rejected_validation);
    const auto issues = manager.validation_issues(raw.profile_id);
    assert(std::find(issues.begin(), issues.end(),
                     core::HardwareProfileIssue::raw_control_namespace) !=
           issues.end());
    assert(std::find(issues.begin(), issues.end(),
                     core::HardwareProfileIssue::protected_ownership) !=
           issues.end());

    auto raw_constraint_entry =
        entry(core::HardwareDomain::display, "display.primary",
              "hardware.display",
              extensions::ExtensionCategory::hardware_display);
    raw_constraint_entry.constraints.push_back(
        {core::SemanticConstraintDomain::variant, "display.pwm.channel"});
    const auto raw_constraint = profile(
        "zie.profile.raw-constraint", {std::move(raw_constraint_entry)});
    assert(manager.declare_profile(
               raw_constraint, profile_authority(raw_constraint.profile_id)) ==
           core::HardwareProfileResult::declared);
    assert(manager.validate_profile(raw_constraint.profile_id) ==
           core::HardwareProfileResult::rejected_validation);

    core::HardwareProfileManager bypass_manager(registry, 3, 8, 2);
    for (const auto& suffix : {"safety-bypass", "raw_gpio", "motor-driver"}) {
      auto bypass_entry =
          entry(core::HardwareDomain::display, "display.primary",
                std::string("hardware.display.") + suffix,
                extensions::ExtensionCategory::hardware_display);
      const auto bypass = profile(std::string("zie.profile.") + suffix,
                                  {std::move(bypass_entry)});
      assert(bypass_manager.declare_profile(
                 bypass, profile_authority(bypass.profile_id)) ==
             core::HardwareProfileResult::declared);
      assert(bypass_manager.validate_profile(bypass.profile_id) ==
             core::HardwareProfileResult::rejected_validation);
    }
  }

  {
    extensions::ExtensionRegistry registry;
    const auto display = hardware_candidate(
        "zie.profile.sdk-display", "hardware.display",
        extensions::ExtensionCategory::hardware_display, "zie.profile.sdk");
    register_active(registry, display);
    core::HardwareProfileManager manager(registry, 2, 8, 2);
    const auto sdk_profile = profile(
        "zie.profile.sdk",
        {entry(core::HardwareDomain::display, "display.primary",
               "hardware.display",
               extensions::ExtensionCategory::hardware_display)});
    declare_and_validate(manager, sdk_profile);
    assert(manager.resolve_profile(sdk_profile.profile_id) ==
           core::HardwareProfileResult::resolved);
    assert(manager.activate_profile(sdk_profile.profile_id) ==
           core::HardwareProfileResult::activated);

    api::AuthoritativeRobotCore authoritative_core;
    api::SemanticRobotApi commands(registry);
    api::ResilientEventBus events(registry, 4,
                                  api::BackpressurePolicy::drop_newest);
    api::RobotStateStore state;
    extensions::TransactionalConfiguration configuration(registry);
    providers::ProviderRouter providers(registry, 4, 2, 4);
    core::ExtensionHost host(registry, authoritative_core, commands, events,
                             state, configuration, providers, &manager);
    auto plugin = std::make_shared<ProfileSdkMock>();
    const auto host_candidate = hardware_candidate(
        "zie.profile.sdk-plugin", "events.read",
        extensions::ExtensionCategory::behavior, sdk_profile.profile_id,
        extensions::ExtensionClass::host_plugin);
    assert(host.declare_extension(host_candidate,
                                  assignment_for(host_candidate),
                                  {{1, 0, 0}, {1, 0, 0}}, plugin) ==
           core::ExtensionHostResult::declared);
    assert(host.validate_extension(host_candidate.manifest.id, {"events.read"}) ==
           core::ExtensionHostResult::validated);
    assert(host.initialize_extension(host_candidate.manifest.id) ==
           core::ExtensionHostResult::initialized);
    assert(host.activate_extension(host_candidate.manifest.id, {"events.read"}) ==
           core::ExtensionHostResult::activated);
    assert(plugin->active->hardware_profile_id() ==
           manager.active_profile()->identity.profile_id);

    auto wrong_plugin = std::make_shared<ProfileSdkMock>();
    const auto wrong_profile_plugin = hardware_candidate(
        "zie.profile.wrong-sdk-plugin", "semantic.presentation",
        extensions::ExtensionCategory::behavior, "zie.profile.not-active",
        extensions::ExtensionClass::host_plugin);
    assert(host.declare_extension(wrong_profile_plugin,
                                  assignment_for(wrong_profile_plugin),
                                  {{1, 0, 0}, {1, 0, 0}}, wrong_plugin) ==
           core::ExtensionHostResult::declared);
    assert(host.validate_extension(wrong_profile_plugin.manifest.id,
                                   {"semantic.presentation"}) ==
           core::ExtensionHostResult::validated);
    assert(host.initialize_extension(wrong_profile_plugin.manifest.id) ==
           core::ExtensionHostResult::initialized);
    assert(host.activate_extension(wrong_profile_plugin.manifest.id,
                                   {"semantic.presentation"}) ==
           core::ExtensionHostResult::rejected_profile);
  }
}
