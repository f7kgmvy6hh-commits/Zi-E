#include <cassert>
#include <limits>
#include "profiles/BenchMinimalProfile.hpp"
#include "zie/api/SafeRobotCommands.hpp"
#include "zie/core/HardwareProfile.hpp"
#include "zie/core/MotionCommandLifecycle.hpp"
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

  using zie::core::CommandIdentity;
  using zie::core::LifecycleResult;
  using zie::core::MotionCommandLifecycle;
  MotionCommandLifecycle lifecycle(500);
  const CommandIdentity first{1, 10, 1};
  assert(lifecycle.submit(first, 0, 1000) ==
         LifecycleResult::rejected_invalid);
  assert(lifecycle.submit({0, 10, 1}, 100, 1000) ==
         LifecycleResult::rejected_invalid);
  assert(lifecycle.submit(first, 501, 1000) ==
         LifecycleResult::rejected_invalid);
  assert(lifecycle.submit(first, 100, 1000) == LifecycleResult::accepted);
  assert(lifecycle.submit(first, 100, 1099) == LifecycleResult::duplicate);
  assert(lifecycle.poll_expiry(1099) == LifecycleResult::no_change);
  assert(lifecycle.submit({1, 10, 2}, 100, 1099) ==
         LifecycleResult::rejected_busy);
  assert(lifecycle.submit({2, 20, 1}, 100, 1099) ==
         LifecycleResult::rejected_busy);
  assert(lifecycle.poll_expiry(1100) == LifecycleResult::expired);
  assert(!lifecycle.snapshot().active);
  assert(lifecycle.snapshot().state == zie::hal::MotionState::faulted);
  assert(lifecycle.snapshot().stop_reason ==
         zie::hal::StopReason::command_lease_expired);
  assert(lifecycle.submit(first, 100, 1200) ==
         LifecycleResult::rejected_stale);

  const CommandIdentity second{1, 10, 2};
  assert(lifecycle.submit(second, 100, 1200) == LifecycleResult::accepted);
  assert(lifecycle.submit(first, 100, 1201) ==
         LifecycleResult::rejected_stale);
  assert(lifecycle.transition(second, zie::hal::MotionState::completed, 1201) ==
         LifecycleResult::rejected_transition);
  assert(lifecycle.transition(second, zie::hal::MotionState::armed, 1201) ==
         LifecycleResult::transitioned);
  assert(lifecycle.transition(second, zie::hal::MotionState::executing, 1202) ==
         LifecycleResult::transitioned);
  assert(lifecycle.transition(second, zie::hal::MotionState::completed, 1203) ==
         LifecycleResult::transitioned);
  assert(!lifecycle.snapshot().active);
  assert(lifecycle.submit({2, 20, 1}, 100, 1204) ==
         LifecycleResult::accepted);

  MotionCommandLifecycle rollover(100);
  const CommandIdentity near_wrap{1, 1, 0xFFFFFFFFU};
  assert(rollover.submit(near_wrap, 20, 0xFFFFFFF5U) ==
         LifecycleResult::accepted);
  assert(rollover.poll_expiry(5) == LifecycleResult::expired);
  assert(rollover.submit({1, 1, 1}, 20, 6) == LifecycleResult::accepted);

  MotionCommandLifecycle transition_expiry(100);
  const CommandIdentity expiring{3, 30, 1};
  assert(transition_expiry.submit(expiring, 10, 100) ==
         LifecycleResult::accepted);
  assert(transition_expiry.transition(expiring, zie::hal::MotionState::armed,
                                      110) == LifecycleResult::expired);

  MotionCommandLifecycle fault_reason(100);
  const CommandIdentity faulting{4, 40, 1};
  assert(fault_reason.submit(faulting, 50, 200) ==
         LifecycleResult::accepted);
  assert(fault_reason.transition(faulting, zie::hal::MotionState::faulted,
                                 201) == LifecycleResult::rejected_invalid);
  assert(fault_reason.transition(faulting, zie::hal::MotionState::faulted,
                                 201, zie::hal::StopReason::hardware_fault) ==
         LifecycleResult::transitioned);
  assert(fault_reason.snapshot().stop_reason ==
         zie::hal::StopReason::hardware_fault);
}
