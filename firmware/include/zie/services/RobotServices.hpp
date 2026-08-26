#pragma once
#include <string_view>
#include "zie/core/HardwareRegistry.hpp"

namespace zie::services {
class PresenceService {
 public:
  explicit PresenceService(core::HardwareRegistry& hardware) : hardware_(hardware) {}
  hal::Status express(std::string_view expression_id, std::string_view audio_cue_id);
  hal::Status enter_privacy_mode();
 private:
  core::HardwareRegistry& hardware_;
};
class MotionService {
 public:
  explicit MotionService(core::HardwareRegistry& hardware) : hardware_(hardware) {}
  hal::MotionState drive(const hal::DriveIntent& intent);
  hal::Status safe_stop(hal::StopReason reason);
  hal::MotionState stow_for_shutdown();
 private:
  core::HardwareRegistry& hardware_;
};
}  // namespace zie::services
