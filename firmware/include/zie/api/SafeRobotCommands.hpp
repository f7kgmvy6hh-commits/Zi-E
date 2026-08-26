#pragma once
#include <string_view>
#include "zie/services/RobotServices.hpp"

namespace zie::api {
// Behavior-facing facade: no GPIO/PWM/register/vendor API and no raw actuator setpoints.
class SafeRobotCommands {
 public:
  SafeRobotCommands(services::PresenceService& presence, services::MotionService& motion)
      : presence_(presence), motion_(motion) {}
  hal::Status express(std::string_view expression_id, std::string_view audio_cue_id);
  hal::MotionState request_base_motion(float linear_normalized,
                                       float angular_normalized,
                                       std::uint32_t lease_ms);
  hal::Status stop_motion();
  hal::MotionState protected_shutdown_stow();
  hal::Status privacy_on();
 private:
  services::PresenceService& presence_;
  services::MotionService& motion_;
};
}  // namespace zie::api
