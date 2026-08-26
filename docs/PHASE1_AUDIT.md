# Phase 1 Audit

## Scope

Phase 1 is commit `17ff901` on `refactor/modular-hardware-architecture`. The audit
compared its firmware scaffold with the approved architecture, safety rules, tests,
and unresolved freeze gates. Files under `legacy/` were not used.

## Completed and coherent

- Stable capability interfaces isolate behavior code from pins, registers, vendor
  SDKs, buses, and raw actuator commands.
- `HardwareRegistry` and profile validation make required capabilities and controller
  ownership explicit.
- The ESP32-S3 multimedia / STM32 safety-motion ownership split is preserved.
- `SafeRobotCommands` exposes normalized, high-level requests and rejects invalid
  motion input.
- Motion is gated on commissioning and optional safety-sensor health.
- Host stubs and architecture tests exercise missing profiles, commissioning gates,
  invalid requests, safety rejection, and failed safe-stop handling.
- No production profile silently binds a stub driver.

## Gaps carried into Phase 2

- `MotionState` is an enum only; legal transitions are not enforced.
- Motion requests have no command/source/session identity, so duplicate, replayed,
  superseded, and out-of-order requests cannot be distinguished.
- Lease values are passed through but there is no local expiry detector or explicit
  ownership arbiter.
- Receipt/acceptance cannot yet be correlated with later execution reports.
- No heartbeat, controller-reboot/session, protocol-version, integrity, or stale-link
  contract exists for the ESP32-S3 to STM32 boundary.
- No production drivers, commissioning persistence, or hardware-in-loop tests exist.
- Exact pins, buses, actuator configuration, timing limits, and safety thresholds
  remain correctly unfrozen.

## Validation result

Static inspection found the Phase 1 scaffold internally consistent with its stated
scope. A fresh host build could not be run because this environment has no CMake,
CTest, or C++ compiler installed. This is an environment limitation, not a passing
test result; the first Phase 2 commit must retain that validation caveat.

## Phase 1 disposition

Preserve Phase 1. Extend its vendor-neutral boundary rather than introducing board
details. Phase 2 must first make command lifecycle and stale-command behavior
testable before choosing a wire encoding or production hardware binding.
