#pragma once

#include <cstdint>
#include <optional>

#include "zie/hal/HardwareInterfaces.hpp"

namespace zie::core {

struct CommandIdentity {
  std::uint32_t source{0};
  std::uint32_t session{0};
  std::uint32_t sequence{0};
};

bool operator==(const CommandIdentity& lhs, const CommandIdentity& rhs);

enum class LifecycleResult {
  accepted,
  duplicate,
  transitioned,
  expired,
  no_change,
  rejected_invalid,
  rejected_busy,
  rejected_stale,
  rejected_transition,
};

struct MotionCommandSnapshot {
  CommandIdentity identity{};
  hal::MotionState state{hal::MotionState::received};
  std::optional<hal::StopReason> stop_reason;
  bool active{false};
};

// Transport-independent single-owner lifecycle for one motion subsystem.
// The caller supplies monotonic milliseconds; duplicate submissions never renew a
// lease. Wire encoding, clock synchronization, and physical safe-stop actuation are
// deliberately outside this class.
class MotionCommandLifecycle {
 public:
  explicit MotionCommandLifecycle(std::uint32_t maximum_lease_ms);

  LifecycleResult submit(const CommandIdentity& identity,
                         std::uint32_t lease_ms,
                         std::uint32_t now_ms);
  LifecycleResult transition(const CommandIdentity& identity,
                             hal::MotionState next_state,
                             std::uint32_t now_ms,
                             std::optional<hal::StopReason> stop_reason =
                                 std::nullopt);
  LifecycleResult poll_expiry(std::uint32_t now_ms);

  const MotionCommandSnapshot& snapshot() const { return snapshot_; }
  std::uint32_t maximum_lease_ms() const { return maximum_lease_ms_; }

 private:
  static bool valid_identity(const CommandIdentity& identity);
  static bool sequence_is_newer(std::uint32_t candidate,
                                std::uint32_t reference);
  static bool legal_transition(hal::MotionState from,
                               hal::MotionState to);
  static bool terminal(hal::MotionState state);

  std::uint32_t maximum_lease_ms_{0};
  std::uint32_t accepted_at_ms_{0};
  std::uint32_t lease_ms_{0};
  MotionCommandSnapshot snapshot_{};
};

}  // namespace zie::core
