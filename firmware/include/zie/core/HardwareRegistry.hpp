#pragma once
#include "zie/hal/HardwareInterfaces.hpp"


namespace zie::core {
enum class ControllerOwner { unbound, host_bench, esp32_s3, stm32 };
struct HardwareRegistry {
  hal::Display* display{nullptr};
  ControllerOwner display_owner{ControllerOwner::unbound};
  hal::MotionController* motion{nullptr};
  ControllerOwner motion_owner{ControllerOwner::unbound};
  hal::Battery* battery{nullptr};
  ControllerOwner battery_owner{ControllerOwner::unbound};
  hal::RangeSensor* range_sensor{nullptr};
  ControllerOwner range_sensor_owner{ControllerOwner::unbound};
  hal::Audio* audio{nullptr};
  ControllerOwner audio_owner{ControllerOwner::unbound};
  hal::Camera* camera{nullptr};
  ControllerOwner camera_owner{ControllerOwner::unbound};
  hal::BellyLightMatrix* belly_light_matrix{nullptr};
  ControllerOwner belly_light_matrix_owner{ControllerOwner::unbound};
  hal::HeadMotion* head_motion{nullptr};
  ControllerOwner head_motion_owner{ControllerOwner::unbound};
  hal::Arms* arms{nullptr};
  ControllerOwner arms_owner{ControllerOwner::unbound};
  hal::SafetySensors* safety_sensors{nullptr};
  ControllerOwner safety_sensors_owner{ControllerOwner::unbound};
};
}  // namespace zie::core
