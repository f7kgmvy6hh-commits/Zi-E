#pragma once
#include <string>
#include "zie/hal/HardwareInterfaces.hpp"
namespace zie::drivers::stub {
class BenchDisplay final : public hal::Display {
 public:
  hal::Status show_expression(std::string_view id) override;
  hal::Status set_enabled(bool enabled) override;
  std::string last_expression;
  bool enabled{true};
};
class BenchMotionController final : public hal::MotionController {
 public:
  hal::MotionState request_drive(const hal::DriveIntent& intent) override;
  hal::MotionState request_protected_shutdown_stow() override;
  hal::Status request_safe_stop(hal::StopReason reason) override;
  hal::MotionState state() const override { return state_; }
  void set_stop_succeeds(bool value) { stop_succeeds_ = value; }
  bool commissioned() const override { return commissioned_; }
  void set_commissioned(bool value) { commissioned_ = value; }
 private:
  bool stop_succeeds_{true};
  bool commissioned_{false};
  hal::MotionState state_{hal::MotionState::received};
};
class BenchBattery final : public hal::Battery {
 public:
  hal::BatteryState read_state() const override { return state; }
  hal::BatteryState state{};
};
class BenchRangeSensor final : public hal::RangeSensor {
 public:
  hal::RangeReading read_range() const override { return reading; }
  hal::RangeReading reading{};
};
class BenchAudio final : public hal::Audio {
 public:
  hal::Status play_cue(std::string_view cue_id) override;
  hal::Status stop() override;
  std::string last_cue;
};
class BenchSafetySensors final : public hal::SafetySensors {
 public:
  bool motion_permitted() const override { return permitted; }
  bool healthy() const override { return is_healthy; }
  bool permitted{true};
  bool is_healthy{true};

};
}  // namespace zie::drivers::stub
