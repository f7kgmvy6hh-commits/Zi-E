# STM32 drivers

Concrete MotionController, Battery telemetry, RangeSensor, HeadMotion, Arms, and
SafetySensors adapters belong here. Limits, command leases, interlocks, plausibility,
safe-stop, and actual-state confirmation are enforced on this controller.

Pins, buses, addresses, actuator identities, directions, ranges, voltage classes, and
motor limits are TODO pending schematic work and low-power commissioning. No raw
actuator interface may be exposed across the AI/behavior boundary.

The STM32CubeCLT composition boundary is `firmware/targets/stm32` and remains
`BOARD_BINDING_DISABLED`. No MCU/package/PCB/clock/timer/ADC/encoder/GPIO/CAN/interrupt
or timing choice is made by the boundary.
