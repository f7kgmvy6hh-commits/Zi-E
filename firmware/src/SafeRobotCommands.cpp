#include "zie/api/SafeRobotCommands.hpp"
#include <cmath>
namespace zie::api {
hal::Status SafeRobotCommands::express(std::string_view expression_id,
                                       std::string_view audio_cue_id) {
  if (expression_id.empty()) return hal::Status::invalid_request;
  return presence_.express(expression_id, audio_cue_id);
}
hal::MotionState SafeRobotCommands::request_base_motion(float linear_normalized,
                                                        float angular_normalized,
                                                        std::uint32_t lease_ms) {
  if (!std::isfinite(linear_normalized) || !std::isfinite(angular_normalized) ||
      linear_normalized < -1.0F || linear_normalized > 1.0F ||
      angular_normalized < -1.0F || angular_normalized > 1.0F ||
      lease_ms == 0) {
    return hal::MotionState::rejected;
  }
  return motion_.drive({linear_normalized, angular_normalized, lease_ms});
}
hal::Status SafeRobotCommands::stop_motion() {
  return motion_.safe_stop(hal::StopReason::requested);
}
hal::MotionState SafeRobotCommands::protected_shutdown_stow() {
  return motion_.stow_for_shutdown();
}
hal::Status SafeRobotCommands::privacy_on() {
  return presence_.enter_privacy_mode();
}
}  // namespace zie::api
