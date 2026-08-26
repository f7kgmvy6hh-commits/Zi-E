#include "zie/core/HardwareProfile.hpp"
namespace zie::core {
namespace {
void require(bool present, const char* name, ProfileValidation& result) {
  if (!present) result.missing_required.emplace_back(name);
}
void require_owner(bool present, ControllerOwner actual, ControllerOwner expected,
                   const char* name, ProfileValidation& result) {
  if (present && actual != expected)
    result.missing_required.emplace_back(std::string(name) + " owner");
}

}
const char* profile_name(ProfileId profile) {
  switch (profile) {
    case ProfileId::bench_minimal: return "bench-minimal";
    case ProfileId::stage1: return "stage1";
    case ProfileId::full_prototype: return "full-prototype";
  }
  return "unknown";
}
ProfileValidation validate_profile(ProfileId profile, const HardwareRegistry& hardware) {
  ProfileValidation result;
  require(hardware.display, "Display", result);
  require(hardware.motion, "MotionController", result);
  require(hardware.battery, "Battery", result);
  require(hardware.range_sensor, "RangeSensor", result);
  require(hardware.audio, "Audio", result);
  const auto multimedia_owner = profile == ProfileId::bench_minimal
                                    ? ControllerOwner::host_bench
                                    : ControllerOwner::esp32_s3;
  const auto safety_owner = profile == ProfileId::bench_minimal
                                ? ControllerOwner::host_bench
                                : ControllerOwner::stm32;
  require_owner(hardware.display, hardware.display_owner, multimedia_owner,
                "Display", result);
  require_owner(hardware.audio, hardware.audio_owner, multimedia_owner,
                "Audio", result);
  require_owner(hardware.motion, hardware.motion_owner, safety_owner,
                "MotionController", result);
  require_owner(hardware.battery, hardware.battery_owner, safety_owner,
                "Battery", result);
  require_owner(hardware.camera, hardware.camera_owner, multimedia_owner,
                "Camera", result);
  require_owner(hardware.belly_light_matrix,
                hardware.belly_light_matrix_owner, multimedia_owner,
                "BellyLightMatrix", result);
  require_owner(hardware.head_motion, hardware.head_motion_owner, safety_owner,
                "HeadMotion", result);
  require_owner(hardware.arms, hardware.arms_owner, safety_owner,
                "Arms", result);
  require_owner(hardware.safety_sensors, hardware.safety_sensors_owner,
                safety_owner, "SafetySensors", result);
  require_owner(hardware.range_sensor, hardware.range_sensor_owner, safety_owner,
                "RangeSensor", result);
  if (profile != ProfileId::bench_minimal) {
    require(hardware.safety_sensors, "SafetySensors", result);
  }
  if (profile == ProfileId::full_prototype) {
    require(hardware.camera, "Camera", result);
    require(hardware.belly_light_matrix, "BellyLightMatrix", result);
    require(hardware.head_motion, "HeadMotion", result);
    require(hardware.arms, "Arms", result);
  }
  result.valid = result.missing_required.empty();
  return result;
}
}  // namespace zie::core
