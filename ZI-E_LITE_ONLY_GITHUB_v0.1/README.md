# ZI-E Lite Aluminum v0.1

**Build-now branch:** stationary, fixed-head, aluminum-body ZI-E for software/AI bring-up before buying the expensive mechanical system.

## Current source of truth
- No 3D printed structure.
- No arms, wheels, Pan/Tilt/Lift motors, laser, cliff sensors or motion MCU in this build.
- Fixed/removable head; nominal face tilt 7 degrees upward, adjustable only during assembly.
- Aluminum body sized to preserve future upgrade interfaces.
- Main electronics candidate: **Waveshare ESP32-S3-Touch-LCD-3.5B** (3.5-inch IPS capacitive touch + ESP32-S3R8 + 8 MB PSRAM + 16 MB flash + IMU + RTC + microphone/audio codec + battery management + camera interface).
- Camera: OV5640 on the board camera interface.
- Speaker: included 6 ohm / 1 W speaker.
- Emotion light: one hidden diffused RGB LED.
- Initial power: USB-C. Battery is optional and intentionally purchased later.

## Start here
1. Read `docs/MASTER_SPEC.md`.
2. Print `templates/ZI-E_Lite_v0.1_1to1_A4_CUT_TEMPLATES.pdf` at **100% / Actual Size**.
3. Verify the 50 mm calibration line before tracing any aluminum.
4. Read `docs/ALUMINUM_BUILD_GUIDE.md` before welding.
5. Read `electronics/WIRING_GUIDE.md` before connecting anything.

## Important
This is a **software-first prototype shell**, not the final mobile body. The full v0.3 mechanical architecture remains deferred, not deleted.
