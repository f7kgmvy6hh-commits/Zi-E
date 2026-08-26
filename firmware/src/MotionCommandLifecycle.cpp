#include "zie/core/MotionCommandLifecycle.hpp"

#include <limits>

namespace zie::core {

bool operator==(const CommandIdentity& lhs, const CommandIdentity& rhs) {
  return lhs.source == rhs.source && lhs.session == rhs.session &&
         lhs.sequence == rhs.sequence;
}

MotionCommandLifecycle::MotionCommandLifecycle(
    const std::uint32_t maximum_lease_ms)
    : maximum_lease_ms_(maximum_lease_ms) {}

LifecycleResult MotionCommandLifecycle::submit(
    const CommandIdentity& identity, const std::uint32_t lease_ms,
    const std::uint32_t now_ms) {
  if (!valid_identity(identity) || lease_ms == 0 ||
      maximum_lease_ms_ == 0 ||
      maximum_lease_ms_ >
          static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max()) ||
      lease_ms > maximum_lease_ms_) {
    return LifecycleResult::rejected_invalid;
  }

  poll_expiry(now_ms);

  if (snapshot_.active) {
    if (identity == snapshot_.identity) {
      return LifecycleResult::duplicate;
    }
    if (identity.source != snapshot_.identity.source ||
        identity.session != snapshot_.identity.session) {
      return LifecycleResult::rejected_busy;
    }
    return sequence_is_newer(identity.sequence, snapshot_.identity.sequence)
               ? LifecycleResult::rejected_busy
               : LifecycleResult::rejected_stale;
  } else if (identity.source == snapshot_.identity.source &&
             identity.session == snapshot_.identity.session &&
             !sequence_is_newer(identity.sequence,
                                snapshot_.identity.sequence)) {
    return LifecycleResult::rejected_stale;
  }

  snapshot_.identity = identity;
  snapshot_.state = hal::MotionState::accepted;
  snapshot_.stop_reason.reset();
  snapshot_.active = true;
  accepted_at_ms_ = now_ms;
  lease_ms_ = lease_ms;
  return LifecycleResult::accepted;
}

LifecycleResult MotionCommandLifecycle::transition(
    const CommandIdentity& identity, const hal::MotionState next_state,
    const std::uint32_t now_ms,
    const std::optional<hal::StopReason> stop_reason) {
  if (poll_expiry(now_ms) == LifecycleResult::expired) {
    return LifecycleResult::expired;
  }
  if (!snapshot_.active || !(identity == snapshot_.identity)) {
    return LifecycleResult::rejected_stale;
  }
  if ((next_state == hal::MotionState::faulted) != stop_reason.has_value()) {
    return LifecycleResult::rejected_invalid;
  }
  if (!legal_transition(snapshot_.state, next_state)) {
    return LifecycleResult::rejected_transition;
  }

  snapshot_.state = next_state;
  snapshot_.stop_reason = stop_reason;
  if (terminal(next_state)) snapshot_.active = false;
  return LifecycleResult::transitioned;
}

LifecycleResult MotionCommandLifecycle::poll_expiry(
    const std::uint32_t now_ms) {
  if (!snapshot_.active) return LifecycleResult::no_change;
  const std::uint32_t elapsed_ms = now_ms - accepted_at_ms_;
  if (elapsed_ms < lease_ms_) return LifecycleResult::no_change;

  snapshot_.state = hal::MotionState::faulted;
  snapshot_.stop_reason = hal::StopReason::command_lease_expired;
  snapshot_.active = false;
  return LifecycleResult::expired;
}

bool MotionCommandLifecycle::valid_identity(const CommandIdentity& identity) {
  return identity.source != 0 && identity.session != 0 &&
         identity.sequence != 0;
}

bool MotionCommandLifecycle::sequence_is_newer(const std::uint32_t candidate,
                                               const std::uint32_t reference) {
  const std::uint32_t distance = candidate - reference;
  return distance != 0 && distance < 0x80000000U;
}

bool MotionCommandLifecycle::legal_transition(const hal::MotionState from,
                                              const hal::MotionState to) {
  switch (from) {
    case hal::MotionState::accepted:
      return to == hal::MotionState::armed ||
             to == hal::MotionState::rejected ||
             to == hal::MotionState::faulted;
    case hal::MotionState::armed:
      return to == hal::MotionState::executing ||
             to == hal::MotionState::rejected ||
             to == hal::MotionState::faulted;
    case hal::MotionState::executing:
      return to == hal::MotionState::completed ||
             to == hal::MotionState::faulted;
    case hal::MotionState::received:
    case hal::MotionState::completed:
    case hal::MotionState::rejected:
    case hal::MotionState::faulted:
      return false;
  }
  return false;
}

bool MotionCommandLifecycle::terminal(const hal::MotionState state) {
  return state == hal::MotionState::completed ||
         state == hal::MotionState::rejected ||
         state == hal::MotionState::faulted;
}

}  // namespace zie::core
