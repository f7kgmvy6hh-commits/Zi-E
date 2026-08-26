#include "drivers/stub/BenchHardware.hpp"
namespace zie::drivers::stub {
hal::Status BenchDisplay::show_expression(std::string_view id) {
  if (!enabled || id.empty()) return hal::Status::rejected;
  last_expression = id;
  return hal::Status::ok;
}
hal::Status BenchDisplay::set_enabled(bool value) {
  enabled = value;
  return hal::Status::ok;
}
hal::MotionState BenchMotionController::request_drive(const hal::DriveIntent&) {
  state_ = commissioned_ ? hal::MotionState::accepted
                         : hal::MotionState::rejected;
  return state_;
}
hal::MotionState BenchMotionController::request_protected_shutdown_stow() {
  state_ = commissioned_ ? hal::MotionState::accepted
                         : hal::MotionState::rejected;
  return state_;
}
hal::Status BenchMotionController::request_safe_stop(hal::StopReason) {
  if (!stop_succeeds_) {
    state_ = hal::MotionState::faulted;
    return hal::Status::faulted;
  }
  state_ = hal::MotionState::completed;
  return hal::Status::ok;
}
hal::Status BenchAudio::play_cue(std::string_view cue_id) {
  if (cue_id.empty()) return hal::Status::invalid_request;
  last_cue = cue_id;
  return hal::Status::ok;
}
hal::Status BenchAudio::stop() {
  last_cue.clear();
  return hal::Status::ok;
}
}  // namespace zie::drivers::stub
