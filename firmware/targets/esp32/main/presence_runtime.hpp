#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace zie::presence {

// Software ingestion limits only; none are electrical, timing, flash, or PSRAM facts.
inline constexpr std::size_t kMaxControlMessageBytes = 1024;
inline constexpr std::size_t kMaxIdBytes = 48;
inline constexpr std::size_t kMaxCapabilities = 12;
inline constexpr std::size_t kMaxDiagnosticBytes = 160;
inline constexpr std::size_t kMaxFacePackEntries = 16;
inline constexpr std::size_t kMaxAudioMetadataBytes = 96;

enum class CapabilityState : std::uint8_t { unavailable, unverified, disabled };
enum class LifecycleState : std::uint8_t { booting, safe_presence };
enum class LinkState : std::uint8_t { disconnected, connected_no_authority };
enum class NetworkState : std::uint8_t { disabled };
enum class FaceMode : std::uint8_t { online_controlled, offline_autonomous };
enum class SafetySemantic : std::uint8_t { stop, freeze, emergency_stop };

struct ProtocolEnvelope {
  std::uint16_t protocol_version{1};
  std::array<char, 24> message_type{};
  std::array<char, kMaxIdBytes> session_id{};
  std::array<char, kMaxIdBytes> generation_id{};
  std::array<char, kMaxIdBytes> command_or_request_id{};
  std::array<std::uint8_t, kMaxControlMessageBytes> payload{};
  std::uint16_t payload_size{0};
};

struct FacePackRecord {
  std::array<char, kMaxIdBytes> pack_id{};
  std::array<char, kMaxIdBytes> version{};
  std::uint16_t entry_count{0};
  bool data_only{true};
};

struct SafetyObservation {
  SafetySemantic semantic{SafetySemantic::stop};
  bool authority{false};
  bool physically_delivered{false};
};

struct RuntimeStatus {
  const char* target{"esp32s3"};
  const char* profile{"GENERIC_UNVERIFIED_ESP32S3"};
  LifecycleState lifecycle{LifecycleState::booting};
  LinkState host_link{LinkState::disconnected};
  NetworkState network{NetworkState::disabled};
  FaceMode face_mode{FaceMode::offline_autonomous};
  CapabilityState camera{CapabilityState::unavailable};
  CapabilityState display{CapabilityState::unavailable};
  CapabilityState audio{CapabilityState::unavailable};
  CapabilityState wake{CapabilityState::unverified};
  CapabilityState rgb{CapabilityState::unavailable};
  CapabilityState touch{CapabilityState::unavailable};
  CapabilityState controller_link{CapabilityState::disabled};
  bool physical_authority{false};
  bool motion_api_exposed{false};
};

class PresenceRuntime final {
 public:
  void initialize() noexcept;
  void observe_host(bool present) noexcept;
  SafetyObservation observe_safety_phrase(SafetySemantic semantic) const noexcept;
  const RuntimeStatus& status() const noexcept { return status_; }

 private:
  RuntimeStatus status_{};
};

const char* to_string(CapabilityState state) noexcept;
const char* to_string(LifecycleState state) noexcept;

static_assert(sizeof(ProtocolEnvelope) <= 1280, "protocol envelope must stay bounded");
static_assert(kMaxCapabilities <= 16, "capability advertisement must stay bounded");
static_assert(kMaxFacePackEntries <= 16, "device face cache index must stay bounded");

}  // namespace zie::presence
