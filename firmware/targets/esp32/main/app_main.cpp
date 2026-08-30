#include <cstdint>

#if !defined(ZIE_BOARD_BINDING_DISABLED) && !defined(ZIE_VERIFIED_BOARD_PROFILE_ACTIVE)
#error "A verified board profile or explicitly disabled generic target is required"
#endif

namespace {

enum class CapabilityState : std::uint8_t {
  unavailable,
  unverified,
  disabled,
};

struct PresenceCapabilities {
  CapabilityState audio{CapabilityState::unverified};
  CapabilityState camera{CapabilityState::unverified};
  CapabilityState display{CapabilityState::unverified};
  CapabilityState wake{CapabilityState::unverified};
  CapabilityState controller_link{CapabilityState::disabled};
  bool production_run_allowed{false};
  bool physical_authority{false};
};

[[maybe_unused]] constexpr PresenceCapabilities kGenericUnverifiedCapabilities{};

}  // namespace

extern "C" void app_main() {
  // Compile-smoke target only. No GPIO, peripheral initialization, network authority,
  // controller-link transmit, flash workflow, or production run mode exists here.
  static_assert(!kGenericUnverifiedCapabilities.production_run_allowed);
  static_assert(!kGenericUnverifiedCapabilities.physical_authority);
}
