#include "zie/services/RobotServices.hpp"
namespace zie::services {
hal::Status PresenceService::express(std::string_view expression_id,
                                     std::string_view audio_cue_id) {
  if (!hardware_.display) return hal::Status::unavailable;
  const auto display_status = hardware_.display->show_expression(expression_id);
  if (display_status != hal::Status::ok) return display_status;
  if (!hardware_.audio || audio_cue_id.empty()) return hal::Status::ok;
  return hardware_.audio->play_cue(audio_cue_id);
}
hal::Status PresenceService::enter_privacy_mode() {
  if (hardware_.audio) hardware_.audio->stop();
  if (!hardware_.camera) return hal::Status::ok;
  return hardware_.camera->set_enabled(false);
}
hal::MotionState MotionService::drive(const hal::DriveIntent& intent) {
  if (!hardware_.motion) return hal::MotionState::rejected;
  if (!hardware_.motion->commissioned()) return hal::MotionState::rejected;
  if (hardware_.safety_sensors &&
      (!hardware_.safety_sensors->healthy() ||
       !hardware_.safety_sensors->motion_permitted())) {
    const auto stop = hardware_.motion->request_safe_stop(
        hal::StopReason::safety_sensor);
    return stop == hal::Status::ok ? hal::MotionState::rejected
                                  : hal::MotionState::faulted;
  }
  return hardware_.motion->request_drive(intent);
}
hal::Status MotionService::safe_stop(hal::StopReason reason) {
  if (!hardware_.motion) return hal::Status::unavailable;
  return hardware_.motion->request_safe_stop(reason);
}
hal::MotionState MotionService::stow_for_shutdown() {
  if (!hardware_.motion) return hal::MotionState::rejected;
  if (!hardware_.motion->commissioned()) return hal::MotionState::rejected;
  return hardware_.motion->request_protected_shutdown_stow();
}
}  // namespace zie::services
