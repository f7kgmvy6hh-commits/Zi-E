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
- `include/zie/extensions/`: centralized versioned extension manifest and capability-state contracts.
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

## Controller-link session

`link/ControllerLinkSession` is the transport-independent Phase 2B1 boundary. A valid
Hello establishes only a candidate session; motion authority remains unavailable until
the first valid heartbeat proves current liveness. It selects the lower compatible
protocol minor, binds liveness to an expected controller and peer boot session, rejects
duplicate/stale heartbeats without refreshing the deadline, and removes authority on
timeout or peer restart. A restarted peer requires explicit local renegotiation and a
fresh heartbeat, and the immediately retired boot session cannot be reactivated by a
delayed Hello.

Malformed, unexpected-controller, retired-session, and incompatible-version Hellos
are rejected without changing a healthy established session. Only a different boot
session presented as the configured expected peer is treated as a restart fault. This
limits unauthenticated Hello denial of service but is not authenticated replay defense.

The heartbeat timeout is verified configuration supplied by the composition root;
this scaffold does not choose one. Wire framing, payload bounds, integrity checks,
CAN/TWAI mapping, retry policy, and physical safe-stop integration remain Phase 2B2
and Phase 2C work.

## Extension manifest foundation

`extensions/ExtensionManifest` separates `HOST_PLUGIN`, `ASSET_PACK`,
`EMBEDDED_MODULE`, and `PROTECTED_SAFETY_MODULE` validation. Trust is supplied by the
future registry rather than claimed by a manifest. Typed permissions are requests, not
runtime grants; embedded and protected modules cannot request host permissions, and
asset packs cannot contain an entrypoint or permissions.

Declared, validated, and active capabilities are distinct sets. Validation requires
each set to be a subset of the preceding trust stage and prevents inactive, failed,
quarantined, disabled, or removed extensions from exposing active capabilities. This
is an in-memory contract only; loading, schema parsing, sandboxing, registry behavior,
and permission enforcement are not implemented.

All enum-bearing manifest, trust-context, and capability-state inputs are checked
against explicit supported domains before class/lifecycle semantics. Out-of-range enum
values fail closed; category trust boundaries do not rely on enum ordinal ranges.

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
