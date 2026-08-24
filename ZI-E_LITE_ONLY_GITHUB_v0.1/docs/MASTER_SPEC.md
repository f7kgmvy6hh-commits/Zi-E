# ZI-E Lite Aluminum v0.1 - Master Specification

## 1. Goal
Build the cheapest useful physical ZI-E first: a stationary AI companion with face, touch, vision, voice, Wi-Fi/BLE, RTC/IMU, and one hidden RGB emotion light. Software is completed and validated before the expensive motion system is purchased.

## 2. Architecture change from the earlier discrete Lite plan
The engineering loop found a simpler integrated option: the Waveshare ESP32-S3-Touch-LCD-3.5B combines the 3.5-inch capacitive display, ESP32-S3R8, 8 MB PSRAM, 16 MB flash, IMU, RTC, microphone/audio codec, battery management, speaker connector, TF slot and OV5640-compatible camera interface on one 92.44 x 61.00 mm board. This eliminates the original separate TFT + ESP32 board + IMU + mic + audio amplifier wiring for the Lite build.

## 3. Nominal envelope
- Body: 126 mm tall, 158 mm bottom width -> 146 mm top width, 108 mm bottom depth -> 98 mm top depth.
- Head: 90 mm tall, 128 mm top width -> 120 mm bottom width, 40 mm deep.
- Stationary base plate: 170 x 120 x 2.0 mm.
- Fixed head angle: nominal 7 degrees upward.
- Estimated bare aluminum mass: about 0.7-0.8 kg before electronics, fasteners and battery.

## 4. Material
Preferred shell: 5052-H32 aluminum, 1.5 mm nominal. 5052 is preferred for enclosure work because it bends without cracking as readily as 6061 and is readily welded/machined. If 1.5 mm is not locally available, 1.3 mm 5052-H32 is acceptable for this stationary Lite shell. Base plate: 2.0 mm aluminum.

## 5. Head / screen
Board envelope: 92.44 x 61.0 x 11.5 mm. Active area: 73.44 x 48.96 mm. Front opening is intentionally 75.5 x 51.0 mm, slightly larger than the active pixels but still inside the black glass border.

Board mounting pattern from the published Waveshare mechanical drawing: four M2 mounting points, approximately 72.00 mm horizontal spacing and 48.50 mm vertical spacing, centered in the 92.44 x 61.00 mm board. Use a removable inner carrier and slotted holes; do not weld the board directly into the head.

## 6. Camera
A 16 x 12 mm top-center camera window is reserved above the display. Candidate A is the OV5640 module bundled/supported by Waveshare. Autofocus is optional only if the exact module pinout is confirmed compatible. The camera window is intentionally larger than the lens so the shell is not tied to one lens housing.

## 7. RF - critical aluminum-body fix
An all-aluminum head can severely attenuate the onboard 2.4 GHz antenna. The design therefore reserves a non-metallic RF window on the head rear/right area. First test the onboard antenna with this window. If range is poor, the board has an IPEX connector and can be switched to an external 2.4 GHz antenna according to the manufacturer procedure. Do not close the antenna completely behind metal.

## 8. Audio
Use the onboard microphone first and the included 6 ohm / 1 W speaker. The head includes an acoustic vent zone; the body bottom includes a hidden speaker outlet. If microphone pickup is weak, add a short acoustic channel or an external MEMS mic later without changing the shell.

## 9. Emotion RGB
Use one cheap diffused common-cathode RGB LED. Suggested first wiring: GPIO39 -> 330 ohm -> Red, GPIO40 -> 330 ohm -> Green, GPIO41 -> 330 ohm -> Blue, common cathode -> GND. PWM controls brightness/color. The front body uses seven 1.2 mm micro-holes plus an internal diffuser/baffle so the light is subtle when off.

## 10. Power
Phase 1: power only by USB-C while software is being developed.
Phase 2 battery candidate: protected 1S 3.7 V LiPo, 4000 mAh nominal, maximum preferred envelope 95 x 55 x 12 mm, connected only to the board's MX1.25 2-pin battery header after connector polarity is verified. The body reserves 100 x 60 x 15 mm. A 5000 mAh pack is acceptable only if it physically fits and uses the correct protected 1S chemistry/connector.

The future full mobile robot still uses the separate higher-voltage traction battery architecture. This 1S Lite battery is not the future wheel/arm traction pack.

## 11. Upgrade interfaces preserved
- Bottom: 4 x M4 future base pattern, 120 x 70 mm. The stationary foot plate can later be removed and replaced by a moving chassis.
- Top: 4 x M4 head-module pattern, 50 x 30 mm + 22 mm central cable pass. The fixed neck can later be removed and replaced by Pan/Tilt/Lift.
- Belly: internal free zone is preserved so the single RGB can later become the hidden matrix.
- Upper side body: shoulder keep-out zones remain uncut for future arms.

## 12. Freeze / do-not-freeze
Frozen for the shell: overall Lite envelope, removable rear panels, board clearance, RF window requirement, bottom/top upgrade interfaces.
Not frozen until parts are in hand: exact camera bracket, mic acoustic duct, speaker clip/holes, battery strap spacing, USB panel extension, final cosmetic finish.
