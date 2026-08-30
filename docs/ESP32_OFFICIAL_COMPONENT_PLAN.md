# ESP32 Official Component Plan

Status: development dependency evaluation; no component or production SDK version is
frozen, downloaded, or vendored. Hardware bindings remain disabled.

| Capability | Preferred source | Evaluation range | License review | Zi-E wrapper | Hardware dependency | Pre-HW-002 build |
|---|---|---|---|---|---|---|
| Wake/AFE/AEC/NR/AGC | `espressif/esp-sr` | donor-tested `~2.4.7`; re-resolve with selected IDF | Component-specific review required | `WakeDetector` / DSP capability adapter | microphones, channel/reference topology, memory | Interface only; DSP deferred |
| Camera | `espressif/esp32-camera` | donor `^2.1.6` | Apache-style/component notice to verify | `CameraDevice` | exact sensor, bus, GPIO, clock, PSRAM | Component resolution possible; init disabled |
| Alternate video stack | `espressif/esp_video` | donor `^2.3.0` | Component-specific review required | same Camera abstraction | verified controller/sensor need | Deferred unless board requires it |
| Codec primitives | `espressif/esp_audio_codec` | donor `~2.5.0` | Component-specific review required | `AudioEncoder/Decoder` | negotiated format | Candidate buildable |
| Codec device | `espressif/esp_codec_dev` | donor `~1.5.6` | Component-specific review required | `AudioCapture/Playback` driver | exact codec/I2S/I2C binding | Init deferred |
| Audio effects | `espressif/esp_audio_effects` | donor `~1.3.0` | Component-specific review required | DSP capability adapter | audio topology and measured performance | Deferred |
| Renderer | `lvgl/lvgl`, `esp_lvgl_port` | donor `~9.5.0`, `~2.8.0` | MIT plus port notice review | Zi-E Face render backend | controller/resolution/bus/memory | Model/build boundary only |
| RGB | `espressif/led_strip` | donor `~3.0.2` | Component-specific review required | semantic RGB effects | HW-008 identity, voltage/timing | Init deferred |

Development toolchain candidate: latest stable official ESP-IDF 6.0.x, tested in a
matrix against 5.5.2 before repository lock. This is not a physically verified board
release. Selection requires official component resolution, ESP32-S3 generic no-pin
smoke build, then HW-002 identity/flash/PSRAM/camera/display/audio comparison.
