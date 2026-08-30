# Zi-E

Zi-E is a modular desktop AI companion robot under active engineering development.
The repository contains the HostRuntime and Control Center foundations, bounded
semantic protocols, a physically inert ESP32-S3 Presence target, host-testable
firmware contracts, hardware evidence, and safety/commissioning gates.

## Current status

- Control Center: App 0.05, evidence-driven engineering cockpit.
- HostRuntime: development foundation and tests are active.
- ESP32-S3: the official ESP-IDF v6.0.2 generic Presence target cross-build passes.
- ESP32 physical binding: disabled; HW-002 has not arrived and is unverified.
- ESP32 flashing: not authorized and absent from the generic build workflow.
- STM32: official toolchain and physical target remain incomplete.
- Phase2B2: WAITING_FOR_VERIFIED_INPUTS.
- Physical commissioning: NOT_STARTED; first integrated power is NOT_AUTHORIZED.

See [Current State](docs/CURRENT_STATE.md) for the exact software and hardware truth.

## Architecture

```text
HostRuntime / AI brain
        authenticated, bounded semantic protocol
ESP32-S3 Presence Runtime
        bounded semantic controller-link boundary
STM32 Safety / Motion Authority
```

HostRuntime owns AI, planning, memory, providers, Hermes integration, semantic
coordination, and the Control Center. ESP32 owns the future network/media/face edge
but no final motion authority. STM32 owns motors, actuators, encoders, limits,
cliff/bumper/current supervision, E-stop, Motion Kill, safe stop, and final physical
motion acceptance or rejection.

A connection never grants motion authority. ACCEPTED does not mean EXECUTED. Face
behavior cannot originate movement. STOP/FREEZE/EMERGENCY_STOP remain deterministic,
bounded safety observations; physical stop confirmation belongs to the protected
STM32 path.

## Quick start

Windows setup creates a local virtual environment and a random bearer token in the
Git-ignored .env file:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\setup.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\start.ps1
```

The Control Center opens on the configured loopback address (default
http://127.0.0.1:8766/). The bearer token stays in page memory only; it is not stored
by the browser.

Run the App and script test suite:

```powershell
.\.venv\Scripts\python.exe -m pytest tests\app tests\scripts -q
.\.venv\Scripts\python.exe .\scripts\run_tests_fallback.py
```

Run the host firmware configure/build/tests with the fixed workflow:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\firmware-workflow.ps1 -Action host-configure
powershell -ExecutionPolicy Bypass -File .\scripts\firmware-workflow.ps1 -Action host-test
```

Run the official generic ESP32-S3 build from an EIM-activated ESP-IDF environment:

```powershell
cd "C:\Users\mohamad alayan\Desktop\Zi-E"
eim run "powershell -ExecutionPolicy Bypass -File .\scripts\build-esp32.ps1"
```

This fixed action is **BUILD ONLY / NO FLASH / GENERIC_UNVERIFIED_ESP32S3**. It
accepts no board profile, port, credentials, flash action, or arbitrary command. A
successful build does not establish compatibility with HW-002 or any physical board.

## Documentation

- [Current State](docs/CURRENT_STATE.md) — concise active status and blockers.
- [Master Specification](docs/ZI-E_MASTER_SPEC.md) — product/architecture contract.
- [Modular Hardware Architecture](docs/MODULAR_HARDWARE_ARCHITECTURE.md) — authority
  and subsystem boundaries.
- [Open Items and Freeze Gates](docs/OPEN_ITEMS_AND_FREEZE_GATES.md) — evidence still
  required.
- [Engineering Method and Memory Loop](docs/ENGINEERING_METHOD_AND_MEMORY_LOOP.md) —
  mandatory review method.
- [Repository Map](REPOSITORY_MAP.md) — active and historical areas.
- [Firmware guide](firmware/README.md) and
  [ESP32 target guide](firmware/targets/esp32/README.md).

## Repository areas

- app/ — HostRuntime-facing Control Center, providers, models, and security bounds.
- firmware/ — shared contracts, host tests, and disabled physical targets.
- mechanical/ and electronics/ — active engineering artifacts and evidence.
- docs/ — current specifications, decisions, evidence, risks, and archives.
- scripts/ — fixed setup, validation, and build workflows.
- legacy/ — preserved historical snapshots/exports and untrusted reference material.

The root README is the human entrypoint, REPOSITORY_MAP.md maps paths, and
PROJECT_CONTEXT.md holds stable product context. Historical exports may preserve
contradictory old facts and never override active evidence.

## Hardware and safety

HW-002 is ordered, has not arrived, and remains unverified. The separately
bench-validated HW678 spare board is not HW-002; its memory, GPIO, power behavior,
and temporary bench wiring do not define a production profile. No camera, display,
audio, touch, RGB, wake, I2C, I2S, TWAI, power, or actuator pin is frozen by the
generic build.

No unrestricted physical motion or hardware API is exposed to AI, App, plugin, or
browser code. No integrated power or normal autonomous motion is permitted until the
physical evidence and low-power commissioning gates pass, including actual-state
confirmation.

## Provenance

XiaoZhi is a pinned engineering donor/reference, not Zi-E's identity, cloud
dependency, or motion-authority source. Imported code/assets require explicit
license and provenance review. Materials under legacy/ are historical and untrusted
unless independently promoted through the current engineering process.
