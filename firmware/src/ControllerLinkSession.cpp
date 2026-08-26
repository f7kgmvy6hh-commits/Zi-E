#include "zie/link/ControllerLinkSession.hpp"

#include <algorithm>
#include <limits>

namespace zie::link {

ControllerLinkSession::ControllerLinkSession(
    const std::uint32_t expected_peer_controller_id,
    const ProtocolVersion local_version,
    const std::uint32_t heartbeat_timeout_ms)
    : expected_peer_controller_id_(expected_peer_controller_id),
      local_version_(local_version),
      heartbeat_timeout_ms_(heartbeat_timeout_ms) {}

LinkResult ControllerLinkSession::accept_hello(const PeerHello& hello,
                                               const std::uint32_t now_ms) {
  if (expected_peer_controller_id_ == 0 || local_version_.major == 0 ||
      heartbeat_timeout_ms_ == 0 ||
      heartbeat_timeout_ms_ >
          static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max()) ||
      hello.controller_id == 0 || hello.boot_session_id == 0 ||
      hello.version.major == 0) {
    return LinkResult::rejected_invalid;
  }
  if (hello.controller_id != expected_peer_controller_id_) {
    return LinkResult::rejected_unexpected_peer;
  }
  if (hello.version.major != local_version_.major) {
    return LinkResult::rejected_incompatible;
  }
  if (hello.boot_session_id == retired_peer_boot_session_id_) {
    return LinkResult::rejected_session;
  }
  if (snapshot_.state == LinkState::active) {
    if (hello.boot_session_id == snapshot_.peer_boot_session_id) {
      return LinkResult::duplicate;
    }
    fault(LinkFault::peer_restarted);
    return LinkResult::peer_restart_detected;
  }
  if (snapshot_.state == LinkState::faulted) {
    return LinkResult::rejected_not_active;
  }

  snapshot_.state = LinkState::active;
  snapshot_.fault.reset();
  snapshot_.peer_boot_session_id = hello.boot_session_id;
  snapshot_.last_heartbeat_sequence = 0;
  snapshot_.negotiated_minor =
      std::min(local_version_.minor, hello.version.minor);
  snapshot_.motion_authority_available = false;
  last_heartbeat_at_ms_ = now_ms;
  return LinkResult::activated;
}

LinkResult ControllerLinkSession::observe_heartbeat(
    const Heartbeat& heartbeat, const std::uint32_t now_ms) {
  if (poll(now_ms) == LinkResult::expired) return LinkResult::expired;
  if (snapshot_.state != LinkState::active) {
    return LinkResult::rejected_not_active;
  }
  if (heartbeat.controller_id != expected_peer_controller_id_ ||
      heartbeat.boot_session_id != snapshot_.peer_boot_session_id) {
    return LinkResult::rejected_session;
  }
  if (heartbeat.sequence == 0) return LinkResult::rejected_invalid;
  if (heartbeat.sequence == snapshot_.last_heartbeat_sequence) {
    return LinkResult::duplicate;
  }
  if (snapshot_.last_heartbeat_sequence != 0 &&
      !sequence_is_newer(heartbeat.sequence,
                         snapshot_.last_heartbeat_sequence)) {
    return LinkResult::rejected_stale;
  }

  snapshot_.last_heartbeat_sequence = heartbeat.sequence;
  last_heartbeat_at_ms_ = now_ms;
  snapshot_.motion_authority_available = true;
  return LinkResult::accepted;
}

LinkResult ControllerLinkSession::poll(const std::uint32_t now_ms) {
  if (snapshot_.state != LinkState::active) return LinkResult::no_change;
  if (now_ms - last_heartbeat_at_ms_ < heartbeat_timeout_ms_) {
    return LinkResult::no_change;
  }
  fault(LinkFault::heartbeat_expired);
  return LinkResult::expired;
}

void ControllerLinkSession::begin_renegotiation() {
  if (snapshot_.peer_boot_session_id != 0) {
    retired_peer_boot_session_id_ = snapshot_.peer_boot_session_id;
  }
  snapshot_ = {};
  last_heartbeat_at_ms_ = 0;
}

bool ControllerLinkSession::sequence_is_newer(
    const std::uint32_t candidate, const std::uint32_t reference) {
  const std::uint32_t distance = candidate - reference;
  return distance != 0 && distance < 0x80000000U;
}

void ControllerLinkSession::fault(const LinkFault reason) {
  snapshot_.state = LinkState::faulted;
  snapshot_.fault = reason;
  snapshot_.motion_authority_available = false;
}

}  // namespace zie::link
