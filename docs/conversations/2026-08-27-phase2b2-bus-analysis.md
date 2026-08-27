# 2026-08-27 — Phase 2B2 Bus/Timing Analysis

## User direction

- Resume from validated commit `3008987` without redoing Phase 2A or Phase 2B1.
- Perform required bus-load and timing analysis before any Phase 2B2 implementation.
- Do not invent physical/timing/framing/integrity values; implement only if evidence
  clears the freeze gate.

## Result

- Verified the approved ESP32-S3 to STM32G0B1 Classical CAN/TWAI architecture against
  repository decisions and primary vendor specifications.
- Derived conservative standard/extended Classical CAN frame-length and serialization
  formulas, explicitly separating sensitivity examples from selected values.
- Found no verified transceiver/topology/clock/bitrate, complete traffic rates/deadlines,
  arbitration priorities, measured queue/ISR latency, safe-stop deadline, fragmentation
  bounds, or integrity/authentication decision.
- Existing 12-byte semantic identities/Hello/Heartbeat cannot fit a single 8-byte
  Classical CAN payload without identifier packing or fragmentation. Neither approach
  is frozen because it changes arbitration, replay, reassembly, and integrity behavior.
- Phase 2B2 implementation was therefore stopped at the documented freeze gate. No
  firmware or wire-format code was added.

## Validation

- Arithmetic and source/provenance review completed.
- Fresh MSYS2 UCRT64 configuration and C++17 build passed using GNU g++ 16.1.0,
  CMake/CTest 4.3.3, and Ninja 1.13.2. CTest passed 1/1 with zero failures.
- No transport, physical bus, bench, HIL, or commissioning validation is claimed.
