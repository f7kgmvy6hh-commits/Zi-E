# Software Bring-up Order

1. Flash factory/demo firmware and verify display + touch + IMU + RTC + audio.
2. Create ZI-E face renderer (LVGL) in landscape orientation.
3. Add touch events: wake, mute, simple settings, debug overlay.
4. Bring up OV5640 capture and local preview.
5. Bring up microphone capture and speaker playback.
6. Add Wi-Fi provisioning and laptop connection.
7. Implement host ZI-E daemon: WebSocket/HTTP status + camera/audio/session link.
8. Connect speech recognition, TTS, LLM and memory on the laptop.
9. Add the single RGB emotion state output.
10. Add diagnostics: board temperature/power state, Wi-Fi RSSI, camera status, audio status, RTC and battery state.
11. Only after the stationary software is stable, start the mechanical upgrade branch.

The API must be hardware-agnostic so the later full robot can keep the same high-level commands while replacing the stationary backend.
