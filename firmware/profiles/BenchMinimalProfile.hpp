#pragma once
#include "drivers/stub/BenchHardware.hpp"
#include "zie/core/HardwareRegistry.hpp"
namespace zie::profiles {
struct BenchMinimalProfile {
  drivers::stub::BenchDisplay display;
  drivers::stub::BenchMotionController motion;
  drivers::stub::BenchBattery battery;
  drivers::stub::BenchRangeSensor range_sensor;
  drivers::stub::BenchAudio audio;
  core::HardwareRegistry hardware;
  BenchMinimalProfile();
};
}  // namespace zie::profiles
