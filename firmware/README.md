# ZI-E firmware

ZI-E firmware is split by responsibility while sharing stable, vendor-neutral contracts. The ESP32-S3 owns multimedia and presence hardware. The STM32 owns motion and safety-critical hardware. Laptop/phone behavior sends only safe, high-level commands.

## Layers

```text
AI / behavior
    -> api/SafeRobotCommands
    -> services
    -> core/HardwareRegistry (interfaces only)
    -> drivers/{esp32,stm32,stub}
    -> vendor SDK / hardware
```

- `include/zie/hal/`: stable hardware interfaces and shared value types.
- `include/zie/core/`: capability registry and hardware-profile declarations.
- `include/zie/services/`: reusable robot logic that depends only on interfaces.
- `include/zie/api/`: the only intended behavior/AI-facing command surface.
- `drivers/esp32/`: multimedia/presence bindings.
- `drivers/stm32/`: safety/motion bindings.
- `drivers/stub/`: deterministic host/bench implementations.
- `profiles/`: composition roots for `bench-minimal`, `stage1`, and `full-prototype`.
- `tests/`: host-side dependency and safety-boundary checks.

Concrete board drivers translate a stable interface into a vendor SDK. Vendor headers, GPIO, PWM, register access, bus addresses, and board pin identifiers stay inside the relevant driver implementation. Services and the AI-facing API must not include them.

## Motion command lifecycle

`core/MotionCommandLifecycle` is the Phase 2A transport-independent safety contract.
It identifies commands by source/session/sequence, permits only one active owner,
rejects duplicates without extending their lease, rejects stale or in-flight
replacement requests, enforces legal execution-state transitions, and detects lease
expiry using caller-supplied monotonic milliseconds. A transition at or after expiry
fails closed as `faulted` with `command_lease_expired`.

This class reports when a safe stop is required; it does not actuate hardware. The
STM32 integration must poll it from the local safety loop and execute/confirm the
physical stop. Cross-session authority, wire encoding, and link heartbeat behavior
remain Phase 2B work.

## Hardware profiles

- `bench-minimal`: stub display, motion controller, battery, range sensor, and audio.
- `stage1`: requires the baseline plus STM32 safety sensing; camera and belly matrix are optional.
- `full-prototype`: requires the baseline plus camera, belly matrix, head motion, arms, and cliff/collision sensing.

Production profile factories intentionally remain unbound until exact hardware details are verified. Missing required capabilities fail profile validation instead of silently falling back.

## Host validation

```sh
cmake -S firmware -B build/firmware-host
cmake --build build/firmware-host
ctest --test-dir build/firmware-host --output-on-failure
```

This validates architecture and safe-command behavior only. It is not hardware commissioning. Before motion on assembled hardware, follow the repository's low-power commissioning and actual-state-confirmation rules.
