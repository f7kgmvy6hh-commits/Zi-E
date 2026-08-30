# 2026-08-30 — ESP32 peripheral bench evidence and maintenance

Superseding note: HW-008's unpowered state below was accurate for this session but was
superseded by the independent functional bench evidence in
[the 2026-08-31 final bench delta](2026-08-31-final-bench-evidence-delta.md).

Purpose: preserve durable Product Owner evidence and the repository-maintenance
outcome without treating chat as hardware authority.

## Physical corrections and evidence

- HW-002, the intended final camera/Presence board, has **not arrived**. It remains
  ordered and unverified. No actual revision, memory, sensor, pinout, USB topology, or
  power behavior is known.
- The present board is a separate spare: PCB HW678 V0.0.0, module marking Sixspan
  S3-N16R8. Read-only identity reported ESP32-S3 QFN56 rev v0.2, dual core + LP,
  Wi-Fi/Bluetooth 5 LE, 40 MHz crystal, 16 MB quad/3.3 V flash, and 8 MB embedded OPI
  PSRAM. Serial bench firmware, PSRAM, onboard GPIO48 RGB colors/off, GPIO0 BOOT
  input, and reset recovery passed. These observations do not define HW-002.
- The HW678 3V3 rail measured 3.35 V. Its silkscreen pin marked 5Vin measured only
  approximately 0.28–0.30 V in both tested USB power modes and is not a verified 5 V
  output.
- HW-006 MAX9814 passed an independent functional bench on the spare board: power,
  analog bias, ESP32 ADC capture, acoustic response, and gain wiring all responded.
  Bias was approximately 1215–1219 mV; example sound windows were approximately
  393–767 mV peak-to-peak. GAIN-to-3V3 selected the approximately 40 dB mode; AR was
  unconnected. GPIO4 was temporary bench wiring only. Production pin, placement,
  quality calibration, and integrated audio commissioning remain open.
- HW-007 carrier marking is VL53L0/1X V2. Probe address 0x29 and register 0xC0 = 0xEE
  verify VL53L0X silicon identity. Ranging was unstable and included invalid/zero/
  65535-like results; electrical/solder continuity is suspect. The unit is quarantined
  pending rework or replacement, not declared destroyed, and must not be powered in
  this pass. GPIO5/GPIO6 were temporary bench I2C pins only.
- HW-008 WS2812-family breakout is present but unpowered. Functional testing is
  blocked until a verified suitable 5 V source exists; HW678 5Vin must not be used.
- HW-010 remains DO_NOT_POWER_OR_CONNECT.
- HW-017 multimeter is physically present and in active bench use. No unverified
  safety-category claim is promoted.

Temporary GPIO4/5/6/48 observations are evidence records, not production assignments.
No powered hardware action occurred during the repository maintenance pass.

## Software/toolchain evidence retained

- Official ESP-IDF v6.0.2 generic ESP32-S3 configure/compile/link/image/size passed
  from the canonical Windows repository path containing spaces.
- Profile remains GENERIC_UNVERIFIED_ESP32S3 with physical bindings disabled.
- Flash remains NOT_AUTHORIZED and no COM port is selected by the fixed workflow.
- The 2 MB IDF flash-image setting is build configuration only, not HW-002 evidence.

## Maintenance outcomes

- Control Center 0.05 derives software and bench readiness from a bounded,
  schema-validated repository evidence record and fails closed if it is malformed.
- Repository metadata uses a short cache; safety/runtime state is not cached.
- HUD refresh has one request owner with generation/abort guards, hidden-tab
  suppression, bounded event rows, and no persisted browser token.
- CSV exports neutralize spreadsheet formula-leading cells.
- The root README and concise Current State now distinguish software readiness from
  physical readiness.
- Historical bundles, exports, migration manifests, and the former chronological
  Current State were preserved in clearly labeled archive areas.

## Unchanged safety gates

STM32 remains final physical motion/safety authority. First integrated power is not
authorized, physical commissioning is not started, and Phase2B2 remains
WAITING_FOR_VERIFIED_INPUTS.
