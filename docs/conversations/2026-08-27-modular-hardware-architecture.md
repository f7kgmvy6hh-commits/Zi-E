# Session archive -- modular hardware architecture

## User-visible objective

Refactor ZI-E toward replaceable/addable displays, sensors, motor drivers, audio,
cameras, batteries, actuators, and other hardware without rewriting unrelated robot
logic, while preserving ESP32-S3 multimedia/presence ownership and STM32 safety/motion
ownership.

## Implemented outcome

- Added stable baseline and optional hardware interfaces.
- Added a dependency registry, services, explicit profile validation, deterministic
  bench drivers, and a safe high-level behavior command facade.
- Added host architecture tests and documented production bindings as unresolved.
- Preserved all legacy, snapshots, bundles, old CAD, and historical material.

## Important limitations

No production pins, buses, addresses, voltages, limits, vendor SDK adapters, or CAN
schema were created. Physical commissioning and hardware verification remain required.
