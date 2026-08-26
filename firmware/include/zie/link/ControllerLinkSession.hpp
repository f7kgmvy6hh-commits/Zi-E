#pragma once

#include <cstdint>
#include <optional>

namespace zie::link {

struct ProtocolVersion {
  std::uint16_t major{0};
  std::uint16_t minor{0};
};

enum class LinkState { disconnected, active, faulted };

enum class LinkFault {
  heartbeat_expired,
  peer_restarted,
};

enum class LinkResult {
  activated,
  accepted,
  duplicate,
  no_change,
  expired,
  rejected_invalid,
  rejected_incompatible,
  rejected_unexpected_peer,
  rejected_not_active,
  rejected_session,
  rejected_stale,
  peer_restart_detected,
};

struct PeerHello {
  std::uint32_t controller_id{0};
  std::uint32_t boot_session_id{0};
  ProtocolVersion version{};
};

struct Heartbeat {
  std::uint32_t controller_id{0};
  std::uint32_t boot_session_id{0};
  std::uint32_t sequence{0};
};

struct LinkSnapshot {
  LinkState state{LinkState::disconnected};
  std::optional<LinkFault> fault;
  std::uint32_t peer_boot_session_id{0};
  std::uint32_t last_heartbeat_sequence{0};
  std::uint16_t negotiated_minor{0};
  bool motion_authority_available{false};
};

// Transport-independent controller-link guard. The caller supplies monotonic time
// and a verified heartbeat timeout. Hello establishes a candidate session but only
// its first valid heartbeat grants motion authority. A different expected-peer boot
// session faults the link; stray Hellos are rejected without replacing healthy state.
class ControllerLinkSession {
 public:
  ControllerLinkSession(std::uint32_t expected_peer_controller_id,
                        ProtocolVersion local_version,
                        std::uint32_t heartbeat_timeout_ms);

  LinkResult accept_hello(const PeerHello& hello, std::uint32_t now_ms);
  LinkResult observe_heartbeat(const Heartbeat& heartbeat,
                               std::uint32_t now_ms);
  LinkResult poll(std::uint32_t now_ms);
  void begin_renegotiation();

  const LinkSnapshot& snapshot() const { return snapshot_; }

 private:
  static bool sequence_is_newer(std::uint32_t candidate,
                                std::uint32_t reference);
  void fault(LinkFault reason);

  std::uint32_t expected_peer_controller_id_{0};
  ProtocolVersion local_version_{};
  std::uint32_t heartbeat_timeout_ms_{0};
  std::uint32_t last_heartbeat_at_ms_{0};
  std::uint32_t retired_peer_boot_session_id_{0};
  LinkSnapshot snapshot_{};
};

}  // namespace zie::link
