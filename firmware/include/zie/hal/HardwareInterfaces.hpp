#pragma once
#include <cstdint>
#include <optional>
#include <string_view>

namespace zie::hal {
enum class Status { ok, unavailable, invalid_request, rejected, faulted };
enum class MotionState { received, accepted, armed, executing, completed, rejected, faulted };
enum class StopReason { requested, command_lease_expired, safety_sensor, hardware_fault };

struct BatteryState {
  std::optional<float> state_of_charge_percent;
  bool charging{false};
  bool faulted{false};
};
struct RangeReading {
  std::optional<float> distance_m;
  bool valid{false};
  std::uint32_t age_ms{0};
};
struct DriveIntent {
  float linear_normalized{0.0F};
  float angular_normalized{0.0F};
  std::uint32_t lease_ms{0};
};

class Display {
 public:
  virtual ~Display() = default;
  virtual Status show_expression(std::string_view expression_id) = 0;
  virtual Status set_enabled(bool enabled) = 0;
};
class MotionController {
 public:
  virtual ~MotionController() = default;
  virtual MotionState request_protected_shutdown_stow() = 0;
  virtual MotionState request_drive(const DriveIntent& intent) = 0;
  virtual Status request_safe_stop(StopReason reason) = 0;
  virtual MotionState state() const = 0;
  virtual bool commissioned() const = 0;
};
class Battery {
 public:
  virtual ~Battery() = default;
  virtual BatteryState read_state() const = 0;
};
class RangeSensor {
 public:
  virtual ~RangeSensor() = default;
  virtual RangeReading read_range() const = 0;
};
class Audio {
 public:
  virtual ~Audio() = default;
  virtual Status play_cue(std::string_view cue_id) = 0;
  virtual Status stop() = 0;
};
class Camera {
 public:
  virtual ~Camera() = default;
  virtual Status set_enabled(bool enabled) = 0;
  virtual bool available() const = 0;
};
class BellyLightMatrix {
 public:
  virtual ~BellyLightMatrix() = default;
  virtual Status show_pattern(std::string_view pattern_id) = 0;
  virtual Status fail_dark() = 0;
};
class HeadMotion {
 public:
  virtual ~HeadMotion() = default;
  virtual MotionState request_pose(std::string_view safe_pose_id) = 0;
  virtual Status request_safe_stop(StopReason reason) = 0;
};
class Arms {
 public:
  virtual ~Arms() = default;
  virtual MotionState request_named_action(std::string_view safe_action_id) = 0;
  virtual Status request_stow() = 0;
};
class SafetySensors {
 public:
  virtual ~SafetySensors() = default;
  virtual bool motion_permitted() const = 0;
  virtual bool healthy() const = 0;
};
}  // namespace zie::hal
