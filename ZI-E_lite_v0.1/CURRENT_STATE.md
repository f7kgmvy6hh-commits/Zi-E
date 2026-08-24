# ZI-E Lite v0.1 — Current State

This repository is the source of truth for the **Lite, stationary, fixed-head, aluminum-body** ZI-E prototype.

## Frozen for this branch
- No 3D-printed structural body.
- Aluminum shell/body fabricated from cut sheet.
- No arms, wheels, head Pan/Tilt/Lift motors, laser, cliff sensors, proximity array, magnetic tool, or motion MCU.
- Head is fixed/non-motorized; final visual integration can be refined after software bring-up.
- Main integrated electronics candidate: Waveshare ESP32-S3-Touch-LCD-3.5B.
- Camera: OV5640-compatible DVP module.
- Audio: onboard microphone/audio path + included speaker first.
- Emotion light: one hidden diffused RGB LED, not the large belly matrix.
- USB-C first. Battery is optional and purchased later after measured current/runtime.
- Software-first workflow: finish face/vision/audio/network/AI integration before buying the deferred mechanical parts.

## Not frozen until real parts are in hand
- Exact camera bracket and lens opening.
- Microphone acoustic duct and speaker vent geometry.
- Final USB-C extension/port geometry.
- Battery strap/holder spacing.
- Cosmetic surface finish.
- Exact RF-window implementation after Wi-Fi/BLE testing in the aluminum enclosure.

## Upgrade compatibility kept
The body keeps removable head/base interfaces and internal keep-outs so a future motorized head, mobile base, larger belly display and arms can be added without treating this Lite prototype as the final mechanical robot.
