# 2026-08-27 — Extension Manifest Foundation

## Direction

- Preserve the committed CAN/timing evidence and mark Phase 2B2 implementation
  `WAITING_FOR_VERIFIED_INPUTS`.
- Continue one independent hardware-free extensible-platform foundation.
- Preserve four distinct extension classes and declared/validated/active capability
  trust stages without bypassing Safe Robot API or STM32 safety ownership.

## Implemented

- Central versioned C++17 manifest model for the approved category catalog.
- Class-specific rules for host plugins, data-only packs, compile-time embedded modules,
  and built-in protected STM32 safety modules.
- Registry-assigned trust context so manifests cannot self-assert trust.
- Typed permission requests with no raw hardware or safety-bypass vocabulary.
- Lifecycle/failure model and declared/validated/active capability subset enforcement.
- Host tests for version, class/category/target, trust, permission, lifecycle, and
  capability escalation failures.
- Independent review found that C++ out-of-domain enum values bypassed the original
  class switch. The validator now explicitly enumerates and rejects every supported
  class, category, permission, assigned trust, controller target, lifecycle, and
  failure value before semantic checks. Hardware-category classification is exhaustive
  rather than ordinal/range-based.

## Deliberately deferred

- JSON schema/parser, package format, registry, dependencies/conflicts, stable device
  identity, configuration engine, loader/sandbox, permission enforcement, signatures,
  commissioning integration, and all Phase 2B2 wire work.

## Validation

- MSYS2 UCRT64 GNU g++ 16.1.0 C++17 build passed.
- Fresh CMake/CTest 4.3.3 configuration/build with Ninja 1.13.2 passed 1/1, including
  focused adversarial `static_cast` tests for every enum-bearing validation input.
- A second fresh build with `-Wall -Wextra -Wpedantic -Wswitch-enum` also passed without
  diagnostics; CTest again passed 1/1.
- No hardware, transport, package-parser, sandbox, or permission-enforcement validation
  is claimed.
