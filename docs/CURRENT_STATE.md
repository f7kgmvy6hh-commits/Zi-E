# Zi-E current state

Updated: 2026-08-31

This is the concise current source of truth. Historical milestone narratives are
under [docs/archive/status/](archive/status/). Machine-readable Control Center
evidence is bounded and schema-validated from
[ZI-E_PROJECT_EVIDENCE.json](evidence/ZI-E_PROJECT_EVIDENCE.json); a valid file
never grants physical authority.

## Executive status

| Domain | Current state |
|---|---|
| HostRuntime / Control Center | DEVELOPMENT_READY; App 0.05 |
| ESP32 software toolchain | READY (verified in the Product Owner's EIM environment) |
| Generic ESP32-S3 Presence build | PASS — GENERIC_UNVERIFIED_ESP32S3 |
| ESP32 physical target | HW-002 NOT_ARRIVED / UNVERIFIED |
| ESP32 flash | NOT_AUTHORIZED; not performed by the generic workflow |
| STM32 development | INCOMPLETE — official tooling/physical target absent |
| Phase2B2 | WAITING_FOR_VERIFIED_INPUTS |
| First integrated power | NOT_AUTHORIZED |
| Physical commissioning | NOT_STARTED |
| Physical motion authority | NONE in Host and ESP32; STM32 remains final authority |

## Architecture and authority

HostRuntime owns AI, planning, memory, providers, Hermes integration, the Control
Center, and semantic coordination. ESP32-S3 Presence owns the future media/network
edge, Face Engine, wake/audio/camera abstractions, Host link, and bounded semantic
controller-link boundary. Neither can directly command physical actuators.

STM32 is the protected motion/safety authority and must independently accept or
reject motion. STOP/FREEZE/EMERGENCY_STOP observations remain deterministic and
bounded. ACCEPTED is not EXECUTED, transport connection grants no authority, and
Face behavior cannot originate movement.

## Verified software evidence

The first official ESP-IDF v6.0.2 ESP32-S3 generic cross-build passed from the
canonical Windows repository path containing spaces. Configure, compile, link,
bootloader generation, application generation, and size reporting passed using GCC
15.2.0. The fixed build script selects only GENERIC_UNVERIFIED_ESP32S3, selects no
COM port, and performs no flash.

Known build-only footprint:

- application image: 0x27350 bytes;
- total IDF image: approximately 160,471 bytes;
- configured build-only app partition: 0x100000, approximately 85% free;
- DIRAM: approximately 47,249 / 341,760 bytes (13.83%);
- flash code/data: approximately 62,558 / 36,676 bytes;
- RTC SLOW/FAST: 36 / 8,192 and 24 / 8,192 bytes.

The IDF report's 16 KiB/16 KiB IRAM line is genuinely full, but it describes only the
dedicated 0x40374000–0x40378000 instruction slice. The map shows .iram0 text
continuing into the shared D/IRAM address space and the full IRAM/DRAM fit assertions
passing; the size tool accounts that shared use in the 47,249/341,760-byte DIRAM row.
This is expected ESP32-S3 region accounting, not an overflow. It is still a useful
internal-SRAM budget signal and must be revisited as real drivers are added. The generated
--flash-size 2MB value is a conservative **build configuration only**, not physical
evidence for HW-002 and not a production partition decision.

Generic physical bindings remain disabled: camera, display, audio/I2S, I2C
peripherals, touch, RGB, wake implementation, TWAI transmit, GPIO profile, and all
motion/actuator paths. Planned interfaces advertise unavailable/unverified/disabled,
never operational hardware.

## Physical evidence

| Item | Current evidence | Integration boundary |
|---|---|---|
| HW-002 final camera/Presence board | ORDERED / NOT_ARRIVED / UNVERIFIED | no revision, memory, sensor, pin, USB, power, or profile inference |
| SPARE-HW678 V0.0.0 / Sixspan S3-N16R8 | PRESENT; independent board/serial/PSRAM/GPIO48 RGB/GPIO0/RST bench evidence passed; 3V3 measured 3.35 V | not HW-002; no repository evidence explicitly proves it is inventory HW-001; its 16 MB flash/8 MB OPI PSRAM and temporary pins do not define production |
| HW-006 MAX9814 | PRESENT; independent power, bias, ADC, acoustic-response, and gain bench PASS | temporary GPIO4 only; final pin/placement/audio integration NOT_COMMISSIONED |
| HW-007 VL53L0X | identity verified at 0x29 with register 0xC0 = 0xEE | ranging NOT_VERIFIED; electrical/solder condition suspect; QUARANTINED; do not power in this pass |
| HW-008 WS2812 breakout | PRESENT; 5V/DI/GND input and 5V/DO/GND output identified; independent 3.3 V bench RED/GREEN/BLUE/OFF sequence PASS through measured approximately 330 Ω series resistor | DI and RGB channels bench PASS; temporary GPIO7 only; final 5 V operation and integrated display/RGB commissioning NOT_VERIFIED / NOT_COMMISSIONED |
| HW-010 power board | PRESENT but identity/electrical evidence incomplete | DO_NOT_POWER_OR_CONNECT |
| HW-017 multimeter | PRESENT and actively used for bounded bench measurements | exact safety/category claims remain unverified; no inferred mains authorization |

The HW678 pin marked 5Vin measured only approximately 0.28–0.30 V in the tested USB
power modes and is **not** a verified 5 V output. It must not be used as HW-008's final
5 V source. HW-008's successful test used the ESP32 3V3 rail and does not establish
final 5 V compatibility. Temporary GPIO4/5/6/7 and GPIO48 bench connections are not
production assignments.

The procurement snapshot describes HW-001 as a photo-confirmed spare
ESP32-S3-N16R8, but the later HW678 bench record does not explicitly link the tested
physical unit to that inventory row. The evidence overlay therefore uses
`SPARE-HW678` and leaves the HW-001 mapping unresolved rather than inventing it.

The 2026-08-29 canonical purchased-parts CSV is preserved as a
[procurement/intake snapshot](archive/procurement/) rather than active verification
truth. Changing physical evidence is kept in the validated project evidence record
and the conversation/bench archive, so procurement rows cannot self-promote
verification.

## Active blockers and next gates

1. Await HW-002 arrival; capture exact board/revision/module/sensor/connector and
   vendor evidence before selecting any verified board profile or flashing.
2. Install and validate the official STM32 toolchain when intentionally scheduled.
3. Resolve the protected controller-link inputs before Phase2B2: no bitrate, IDs,
   pins, transceiver, watchdog timing, or frame packing is frozen.
4. Rework or replace HW-007 and establish a verified suitable 5 V source before
   HW-008 final-voltage validation or integrated display/RGB commissioning.
5. Keep HW-010 disconnected until identity, polarity, input/output, isolation,
   current, and protection evidence are complete.
6. Run low-power, fail-closed subsystem commissioning before any integrated power
   or normal autonomous motion.
