# STM32 drivers

Concrete MotionController, Battery telemetry, RangeSensor, HeadMotion, Arms, and
SafetySensors adapters belong here. Limits, command leases, interlocks, plausibility,
safe-stop, and actual-state confirmation are enforced on this controller.

Pins, buses, addresses, actuator identities, directions, ranges, voltage classes, and
motor limits are TODO pending schematic work and low-power commissioning. No raw
actuator interface may be exposed across the AI/behavior boundary.
