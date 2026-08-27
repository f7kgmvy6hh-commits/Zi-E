#include <cassert>
#include <limits>
#include "profiles/BenchMinimalProfile.hpp"
#include "zie/api/SafeRobotCommands.hpp"
#include "zie/core/HardwareProfile.hpp"
#include "zie/core/MotionCommandLifecycle.hpp"
#include "zie/link/ControllerLinkSession.hpp"
void run_extension_manifest_tests();
void run_extension_registry_tests();
void run_transactional_configuration_tests();
void run_semantic_robot_api_tests();
void run_event_bus_virtual_robot_tests();
void run_provider_foundation_tests();
void run_presentation_engine_tests();
void run_device_identity_tests();
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
  assert(rollover.poll_expiry(5) == LifecycleResult::no_change);
  assert(rollover.poll_expiry(9) == LifecycleResult::expired);
  assert(rollover.submit({1, 1, 1}, 20, 10) == LifecycleResult::accepted);

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

  using zie::link::ControllerLinkSession;
  using zie::link::Heartbeat;
  using zie::link::LinkFault;
  using zie::link::LinkResult;
  using zie::link::LinkState;
  using zie::link::PeerHello;
  using zie::link::ProtocolVersion;

  ControllerLinkSession link(2, ProtocolVersion{1, 3}, 50);
  assert(link.accept_hello(PeerHello{3, 10, {1, 2}}, 100) ==
         LinkResult::rejected_unexpected_peer);
  assert(link.snapshot().state == LinkState::disconnected);
  assert(!link.snapshot().motion_authority_available);
  assert(link.accept_hello(PeerHello{2, 10, {2, 0}}, 100) ==
         LinkResult::rejected_incompatible);
  assert(link.snapshot().state == LinkState::disconnected);
  assert(!link.snapshot().motion_authority_available);
  assert(link.accept_hello(PeerHello{2, 10, {1, 2}}, 100) ==
         LinkResult::activated);
  assert(link.snapshot().negotiated_minor == 2);
  assert(!link.snapshot().motion_authority_available);
  assert(link.accept_hello(PeerHello{2, 10, {1, 2}}, 101) ==
         LinkResult::duplicate);
  assert(!link.snapshot().motion_authority_available);
  assert(link.observe_heartbeat(Heartbeat{3, 10, 1}, 110) ==
         LinkResult::rejected_session);
  assert(!link.snapshot().motion_authority_available);
  assert(link.observe_heartbeat(Heartbeat{2, 11, 1}, 111) ==
         LinkResult::rejected_session);
  assert(!link.snapshot().motion_authority_available);
  assert(link.observe_heartbeat(Heartbeat{2, 10, 0}, 112) ==
         LinkResult::rejected_invalid);
  assert(!link.snapshot().motion_authority_available);
  assert(link.observe_heartbeat(Heartbeat{2, 10, 1}, 120) ==
         LinkResult::accepted);
  assert(link.snapshot().motion_authority_available);
  assert(link.observe_heartbeat(Heartbeat{2, 10, 1}, 130) ==
         LinkResult::duplicate);
  assert(link.snapshot().motion_authority_available);
  assert(link.observe_heartbeat(Heartbeat{2, 10, 3}, 140) ==
         LinkResult::accepted);
  assert(link.observe_heartbeat(Heartbeat{2, 10, 2}, 141) ==
         LinkResult::rejected_stale);
  assert(link.snapshot().motion_authority_available);
  assert(link.observe_heartbeat(Heartbeat{2, 11, 4}, 142) ==
         LinkResult::rejected_session);
  assert(link.snapshot().motion_authority_available);
  assert(link.accept_hello(PeerHello{3, 99, {1, 2}}, 142) ==
         LinkResult::rejected_unexpected_peer);
  assert(link.snapshot().state == LinkState::active);
  assert(link.snapshot().motion_authority_available);
  assert(link.accept_hello(PeerHello{2, 10, {2, 0}}, 142) ==
         LinkResult::rejected_incompatible);
  assert(link.snapshot().state == LinkState::active);
  assert(link.snapshot().motion_authority_available);
  assert(link.accept_hello(PeerHello{0, 0, {0, 0}}, 142) ==
         LinkResult::rejected_invalid);
  assert(link.snapshot().state == LinkState::active);
  assert(link.snapshot().motion_authority_available);
  assert(link.accept_hello(PeerHello{2, 11, {1, 2}}, 143) ==
         LinkResult::peer_restart_detected);
  assert(link.snapshot().fault == LinkFault::peer_restarted);
  assert(!link.snapshot().motion_authority_available);
  assert(link.accept_hello(PeerHello{2, 11, {1, 2}}, 144) ==
         LinkResult::rejected_not_active);
  link.begin_renegotiation();
  assert(link.accept_hello(PeerHello{2, 10, {1, 2}}, 199) ==
         LinkResult::rejected_session);
  assert(link.accept_hello(PeerHello{2, 11, {1, 3}}, 200) ==
         LinkResult::activated);
  assert(!link.snapshot().motion_authority_available);
  assert(link.observe_heartbeat(Heartbeat{2, 10, 1}, 201) ==
         LinkResult::rejected_session);
  assert(!link.snapshot().motion_authority_available);
  assert(link.observe_heartbeat(Heartbeat{2, 11, 1}, 202) ==
         LinkResult::accepted);
  assert(link.snapshot().motion_authority_available);
  assert(link.accept_hello(PeerHello{2, 10, {1, 2}}, 203) ==
         LinkResult::rejected_session);
  assert(link.snapshot().state == LinkState::active);
  assert(link.snapshot().motion_authority_available);
  assert(link.poll(251) == LinkResult::no_change);
  assert(link.poll(252) == LinkResult::expired);
  assert(link.snapshot().fault == LinkFault::heartbeat_expired);
  assert(!link.snapshot().motion_authority_available);

  ControllerLinkSession duplicate_deadline(2, ProtocolVersion{1, 0}, 50);
  assert(duplicate_deadline.accept_hello(PeerHello{2, 1, {1, 0}}, 100) ==
         LinkResult::activated);
  assert(duplicate_deadline.observe_heartbeat(Heartbeat{2, 1, 1}, 110) ==
         LinkResult::accepted);
  assert(duplicate_deadline.observe_heartbeat(Heartbeat{2, 1, 1}, 159) ==
         LinkResult::duplicate);
  assert(duplicate_deadline.poll(160) == LinkResult::expired);

  ControllerLinkSession link_rollover(2, ProtocolVersion{1, 0}, 20);
  assert(link_rollover.accept_hello(PeerHello{2, 1, {1, 0}},
                                    0xFFFFFFF5U) ==
         LinkResult::activated);
  assert(link_rollover.observe_heartbeat(
             Heartbeat{2, 1, 0xFFFFFFFFU}, 0xFFFFFFF6U) ==
         LinkResult::accepted);
  assert(link_rollover.observe_heartbeat(Heartbeat{2, 1, 1}, 2) ==
         LinkResult::accepted);
  assert(link_rollover.poll(22) == LinkResult::expired);

  run_extension_manifest_tests();
  run_extension_registry_tests();
  run_transactional_configuration_tests();
  run_semantic_robot_api_tests();
  run_event_bus_virtual_robot_tests();
  run_provider_foundation_tests();
  run_presentation_engine_tests();
  run_device_identity_tests();
}
