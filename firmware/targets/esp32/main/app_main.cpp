#include "presence_runtime.hpp"

#include "esp_log.h"

extern "C" void app_main() {
  static constexpr char kTag[] = "zie_presence";
  static zie::presence::PresenceRuntime runtime;
  runtime.initialize();
  const auto& status = runtime.status();
  ESP_LOGI(kTag, "target=%s profile=%s lifecycle=%s physical_authority=%s",
           status.target, status.profile, zie::presence::to_string(status.lifecycle),
           status.physical_authority ? "true" : "false");
  ESP_LOGI(kTag, "camera=%s display=%s audio=%s wake=%s controller_link=%s",
           zie::presence::to_string(status.camera),
           zie::presence::to_string(status.display),
           zie::presence::to_string(status.audio),
           zie::presence::to_string(status.wake),
           zie::presence::to_string(status.controller_link));
}
