#include <cassert>
#include <limits>
#include "profiles/BenchMinimalProfile.hpp"
#include "zie/api/SafeRobotCommands.hpp"
#include "zie/core/HardwareProfile.hpp"
int main() {
  zie::profiles::BenchMinimalProfile profile;
  auto validation = zie::core::validate_profile(
      zie::core::ProfileId::bench_minimal, profile.hardware);
  assert(validation.valid);
  auto full = zie::core::validate_profile(
      zie::core::ProfileId::full_prototype, profile.hardware);
  assert(!full.valid);
  assert(full.missing_required.size() == 10);
  auto stage1 = zie::core::validate_profile(
      zie::core::ProfileId::stage1, profile.hardware);
  assert(!stage1.valid);
  assert(stage1.missing_required.size() == 6);

  zie::services::PresenceService presence(profile.hardware);
  zie::services::MotionService motion(profile.hardware);
  zie::api::SafeRobotCommands commands(presence, motion);
  assert(commands.express("awake", "ready") == zie::hal::Status::ok);
  assert(profile.display.last_expression == "awake");
  assert(profile.audio.last_cue == "ready");
  assert(commands.request_base_motion(0.2F, 0.0F, 100) ==
         zie::hal::MotionState::rejected);
  assert(commands.protected_shutdown_stow() ==
         zie::hal::MotionState::rejected);
  profile.motion.set_commissioned(true);
  assert(commands.request_base_motion(0.2F, 0.0F, 100) ==
         zie::hal::MotionState::accepted);
  zie::drivers::stub::BenchSafetySensors safety;
  assert(commands.protected_shutdown_stow() ==
         zie::hal::MotionState::accepted);
  profile.hardware.safety_sensors = &safety;
  profile.hardware.safety_sensors_owner =
      zie::core::ControllerOwner::host_bench;
  safety.permitted = false;
  assert(commands.request_base_motion(0.2F, 0.0F, 100) ==
         zie::hal::MotionState::rejected);
  profile.motion.set_stop_succeeds(false);
  assert(commands.request_base_motion(0.2F, 0.0F, 100) ==
         zie::hal::MotionState::faulted);
  profile.motion.set_stop_succeeds(true);
  safety.permitted = true;
  assert(commands.request_base_motion(1.1F, 0.0F, 100) ==
         zie::hal::MotionState::rejected);
  assert(profile.motion.state() == zie::hal::MotionState::faulted);
  assert(commands.request_base_motion(
             std::numeric_limits<float>::quiet_NaN(), 0.0F, 100) ==
         zie::hal::MotionState::rejected);
  assert(commands.request_base_motion(0.0F, 0.0F, 0) ==
         zie::hal::MotionState::rejected);
  assert(commands.stop_motion() == zie::hal::Status::ok);
}
