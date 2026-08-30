# Development Pipeline and Physical Integration Readiness — 2026-08-30

This is preparation, not commissioning. SDKs stay outside the repository; no physical
pins, clocks, CAN values, electrical ratings or board revisions are frozen.

## Machine toolchain matrix

| Area | Tool | Status | Detected version/path or required action |
|---|---|---|---|
| Host | Git | INSTALLED | 2.45.1.windows.1 — `C:\Program Files\Git\cmd\git.exe` |
| Host | project Python | INSTALLED | 3.11.16 — `.venv\Scripts\python.exe` |
| Host | project pip | MISSING | venv has no pip executable/module; App dependencies are nevertheless import-ready; rerun the existing official `uv`-based setup to rebuild, do not mutate the validated venv casually |
| Host | system Python/pip | OPTIONAL / INSTALLED | Python 3.14.7 and pip 26.2.1 under `%LOCALAPPDATA%\Python`; not used by the project venv |
| Host | uv | OPTIONAL / INSTALLED | 0.12.7 — existing App setup package manager |
| Host | CMake | INSTALLED | 4.3.3 — `C:\msys64\ucrt64\bin\cmake.exe` |
| Host | Ninja | INSTALLED | 1.13.2 — `C:\msys64\ucrt64\bin\ninja.exe` |
| Host | GNU C/C++ | INSTALLED | MSYS2 UCRT64 16.1.0 — `C:\msys64\ucrt64\bin\gcc.exe`, `g++.exe` |
| Host | PowerShell | INSTALLED | Windows PowerShell 5.1.19041.6456 |
| Host | Node.js | OPTIONAL / INSTALLED | 26.7.0 — used for JS syntax checks |
| Host | Codex | OPTIONAL / INSTALLED | codex-cli 0.151.0 |
| ESP32 | EIM | MANUAL_VENDOR_INSTALL_REQUIRED | not detected; sandbox cannot execute WinGet |
| ESP32 | ESP-IDF / `idf.py` | MANUAL_VENDOR_INSTALL_REQUIRED | not detected; exact version unknown |
| ESP32 | Xtensa ESP32-S3 compiler | MANUAL_VENDOR_INSTALL_REQUIRED | not detected |
| ESP32 | esptool | MANUAL_VENDOR_INSTALL_REQUIRED | not detected |
| ESP32 | OpenOCD | MANUAL_VENDOR_INSTALL_REQUIRED | not detected |
| ESP32 | compatible board | BLOCKED_ON_HARDWARE | `HW-002` is ordered; exact board is not verified |
| STM32 | STM32CubeCLT | MANUAL_VENDOR_INSTALL_REQUIRED | not detected |
| STM32 | managed GNU Arm compiler/GDB | MANUAL_VENDOR_INSTALL_REQUIRED | not detected |
| STM32 | STM32CubeProgrammer CLI | MANUAL_VENDOR_INSTALL_REQUIRED | not detected |
| STM32 | STM32CubeMX | OPTIONAL / MANUAL_VENDOR_INSTALL_REQUIRED | not detected; useful later for reviewed clock/pin generation |
| STM32 | ST-LINK server/driver/probe | MANUAL_VENDOR_INSTALL_REQUIRED / BLOCKED_ON_HARDWARE | no tools, driver evidence, or verified probe detected |

Run `scripts/toolchain-doctor.ps1` for current machine truth. It reports `READY`,
`MISSING`, `OPTIONAL`, `MANUAL_VENDOR_INSTALL_REQUIRED` and `HARDWARE_REQUIRED` without
downloading or running arbitrary content.

## Official manual installs

### Espressif

Automatic install was attempted with the user-specified official WinGet ID but Windows
Apps execution is blocked in this sandbox. In an ordinary non-administrator PowerShell:

```powershell
winget install --id Espressif.EIM-CLI --exact --source winget
eim --help
eim install
eim list
```

Use the EIM-supported latest stable ESP-IDF selection for ESP32-S3, outside Zi-E. Use
the EIM-generated activation environment, then rerun the doctor. Do not copy ESP-IDF,
tools, archives or activation paths into the repository. Official EIM documentation:
`https://docs.espressif.com/projects/idf-im-ui/en/latest/`.

This machine is Windows 10; current EIM documentation says Windows 10 is no longer a
supported EIM platform. If EIM refuses installation, use Espressif's officially
documented supported-host route (for example a supported Windows host or its documented
WSL flow), not an unofficial bundle. The repository records no ESP-IDF version until
`idf.py --version` actually succeeds.

### STMicroelectronics

Download STM32CubeCLT from ST's official STM32 software-tools page, accept its license,
and install outside Zi-E. Ensure the CLT-managed `arm-none-eabi-gcc`,
`arm-none-eabi-gdb`, STM32CubeProgrammer CLI and ST-LINK GDB server are selected. Install
official ST-LINK USB drivers if the package/host requires them. STM32CubeMX is optional
and should be installed only for later reviewed MCU/clock/pin generation. ST licensing,
interactive installer, elevation or authentication must be completed by the user; do
not bypass them. STM32CubeIDE is not required: CubeCLT + CubeMX provides the clearer
CLI/generation boundary needed here.

## One local bootstrap

```powershell
scripts\toolchain-doctor.ps1
scripts\setup.ps1
scripts\firmware-workflow.ps1 -Action host-configure
scripts\firmware-workflow.ps1 -Action host-build
scripts\firmware-workflow.ps1 -Action host-test
scripts\toolchain-doctor.ps1
scripts\start.ps1
```

The doctor and Control Center reconciliation show the exact target/hardware blockers.
There is no second setup system. Target workflows remain disabled until reviewed board
profiles exist under `firmware/board_profiles`.

## Target build and device-selection boundary

- ESP32: configure/build/flash/monitor require a reviewed board profile. Flash/monitor
  additionally require an explicit `COM<number>` that is currently detected. No first
  COM port or discovery-order selection exists.
- STM32: configure/build/program require a reviewed profile. Programming additionally
  requires an exact ST-LINK serial and explicit firmware image. Debug/log remain
  disabled until their transport and physical target are reviewed.
- With no hardware profile, both target CMake boundaries stop with
  `BOARD_BINDING_DISABLED`. Fake pins are neither needed nor accepted.

## Real App/robot boundary

The existing `HostRuntimeAdapter` remains the sole future real-target boundary. A
configuration flag cannot provide availability, session identity, authority generation,
delivery or physical confirmation. The current unavailable adapter stays fail-closed.

Future authenticated transport responsibilities, after commissioning, are:

- Host to ESP32: semantic command request, host/session identity, authoritative
  generation and exact configuration/profile identity.
- ESP32 to Host: copied readiness, telemetry/events, delivery state and controller-link
  state, all bound to the current session/generation.
- ESP32 to STM32: bounded controller-link messages only after Phase2B2 freezes the
  verified wire contract.

No HUD/App endpoint may expose a raw socket, GPIO, PWM, motor register or CAN frame.

## Real-bench progression

| Bench | Scope | Current state |
|---|---|---|
| 0 | PC + toolchains + App/host tests | EXECUTABLE NOW; vendor target tools incomplete |
| 1 | one verified ESP32 over USB, no peripherals | BLOCKED: `HW-002` not received; spare board identity unreviewed |
| 2 | one verified ESP32 peripheral | BLOCKED on Bench 1 and per-item evidence |
| 3 | verified STM32 over exact programmer/debugger | BLOCKED: controller/tool/probe missing |
| 4 | ESP32 + STM32 link, actuator power absent | BLOCKED: Phase2B2 inputs/link hardware missing |
| 5 | sensors individually | BLOCKED on verified controllers/power/interfaces |
| 6 | one bounded restrained actuator | BLOCKED on hardware and prior physical gates |
| 7 | integrated physical commissioning | BLOCKED; zero physical gates passed |

Bench 0 proves software/tool execution only. USB logic power and robot physical power
are separate gates. No robot-wide power is authorized.

## Hardware/evidence still required

- received and reviewed primary ESP32-S3 controller/camera board;
- STM32 safety/motion controller and exact MCU/package/PCB/clock evidence;
- CAN transceivers, verified termination/cabling/topology and controller-link evidence;
- exact ST-LINK or equivalent programmer/debugger if the selected STM32 requires it;
- encoded drive motors, motor drivers, wheels/caster, servos/actuators and mechanisms;
- cliff/collision sensors and verified physical safe-stop/emergency path;
- battery pack, BMS/protection, certified charger, primary fuse/eFuse/branch protection;
- verified DC/DC rails including input/output/polarity/isolation/current/thermal facts;
- reviewed connectors, crimp contacts, harness topology, gauge, keying and strain relief.

Design candidates do not satisfy this list and are not purchased inventory.

## CAD/stability measurement handoff

The next mechanical pass needs measured motor dimensions/mass/shaft position; wheel
diameter/width/runout; actuator mass/mounting axes; battery dimensions/mass; component
positions; arm/tool payload and reach; assembled center of gravity; support polygon;
and measured braking/acceleration/traction assumptions. Record configuration, datum,
method and uncertainty. CAD estimates and simulation do not establish physical
stability; the measured prototype must validate it.

## Readiness statement

- SOFTWARE TOOLCHAIN READY: host only; vendor target tools incomplete.
- HOST TESTS READY: yes.
- ESP32 BUILD READY: boundary ready, toolchain/profile missing, no smoke compile.
- STM32 BUILD READY: boundary ready, toolchain/profile missing, no ARM smoke compile.
- HARDWARE NOT VERIFIED: yes.
- FIRST POWER NOT AUTHORIZED: yes.
- PHYSICAL COMMISSIONING NOT DONE: yes.
- PHASE2B2: `WAITING_FOR_VERIFIED_INPUTS`.
