# Hardware profile bindings

`BenchMinimalProfile` is executable now and binds deterministic stub drivers.

The `stage1` and `full-prototype` requirement sets are implemented by
`core::validate_profile`. Their production composition roots remain explicit TODOs
until verified board details and concrete drivers exist. Stage1 and full-prototype
require STM32 safety sensing. All bindings carry validated ownership as:

- ESP32-S3: Display, Audio, Camera, BellyLightMatrix.
- STM32: MotionController, Battery safety telemetry, safety RangeSensor instances,
  HeadMotion, Arms, SafetySensors.
- The head measurement RangeSensor may be ESP32-S3-owned when used only for
  presence/vision assistance; it must not substitute for STM32 safety sensing.

Do not place guessed pins, addresses, limits, or electrical parameters here.
