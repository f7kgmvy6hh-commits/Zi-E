#include "presence_runtime.hpp"

namespace zie::presence {

void PresenceRuntime::initialize() noexcept {
  status_ = RuntimeStatus{};
  status_.lifecycle = LifecycleState::safe_presence;
}

void PresenceRuntime::observe_host(bool present) noexcept {
  status_.host_link = present ? LinkState::connected_no_authority : LinkState::disconnected;
  status_.face_mode = present ? FaceMode::online_controlled : FaceMode::offline_autonomous;
}

SafetyObservation PresenceRuntime::observe_safety_phrase(SafetySemantic semantic) const noexcept {
  return SafetyObservation{semantic, false, false};
}

const char* to_string(CapabilityState state) noexcept {
  switch (state) {
    case CapabilityState::unavailable: return "UNAVAILABLE";
    case CapabilityState::unverified: return "UNVERIFIED";
    case CapabilityState::disabled: return "DISABLED";
  }
  return "UNAVAILABLE";
}

const char* to_string(LifecycleState state) noexcept {
  switch (state) {
    case LifecycleState::booting: return "BOOTING";
    case LifecycleState::safe_presence: return "SAFE_PRESENCE";
  }
  return "BOOTING";
}

}  // namespace zie::presence
