# ZI-E CAD v0.3 — Head Laser Rangefinder

Primary envelope in this CAD: Adafruit-class VL53L1X carrier, 25.5 × 17.5 × 4.6 mm, placed to the right of the centered OV5640 camera. Final design is expected to use the much smaller bare VL53L1X or a compact carrier.

## Optical rules
- 940 nm invisible Class 1 laser.
- Dedicated IR-qualified brow region; do not assume cosmetic smoked plastic is transparent enough.
- Target >85% material transmission around 930–950 nm per ST cover-window guidance.
- Minimize sensor-to-window air gap and window angle.
- Add black optical isolation/gasket between emitter/receiver paths and surrounding cavity.
- Perform offset and crosstalk calibration after the final brow/window is assembled.

## Mechanical intent
- Camera remains centered.
- Microphone envelopes remain symmetric.
- Prototype breakout fits without forcing final head size; final custom PCB should shrink.
- The sensor is not used as a structural element or as the sole collision/cliff safety sensor.

## Verification
See repository `docs/LASER_RANGEFINDER_SPEC.md`.
