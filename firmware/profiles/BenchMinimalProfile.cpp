#include "profiles/BenchMinimalProfile.hpp"
namespace zie::profiles {
BenchMinimalProfile::BenchMinimalProfile() {
  hardware.display = &display;
  hardware.display_owner = core::ControllerOwner::host_bench;
  hardware.motion = &motion;
  hardware.motion_owner = core::ControllerOwner::host_bench;
  hardware.battery = &battery;
  hardware.battery_owner = core::ControllerOwner::host_bench;
  hardware.range_sensor = &range_sensor;
  hardware.range_sensor_owner = core::ControllerOwner::host_bench;
  hardware.audio = &audio;
  hardware.audio_owner = core::ControllerOwner::host_bench;
}
}  // namespace zie::profiles
