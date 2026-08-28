# Component Master Matrix

This is the quick lookup table. `Primary` means current first-choice; `Backup` means an intentional substitute, not an equal recommendation. Prices are deliberately omitted because they are volatile; re-check before purchasing.

This is a design-choice quick reference, not physical inventory. The canonical
reconciliation/status/provenance layer is
`HARDWARE_SOURCE_OF_TRUTH_RECONCILIATION.md`; purchased items must enter through
`PURCHASED_PARTS_INTAKE_SCHEMA.md`. `KEEP` below never means physically verified.

| Subsystem | Primary | Backup(s) / Switch Condition | Status |
|---|---|---|---|
| Display | 3.5" IPS touch, ILI9488, 480×320 landscape | EastRising/BuyDisplay or equivalent documented panel if FPC/touch IC/availability fails | KEEP, verify exact panel |
| Belly Light Matrix | IS31FL3741 13×9 RGB prototype + secret-until-lit optical insert | Wider custom IS31FL3741-class matrix after optical/power test | KEEP architecture, VERIFY optics/thermal |
| Camera | OV5640 5MP AF | OV2640 if budget forces it | KEEP |
| Head laser rangefinder | VL53L1X 940 nm Class-1 ToF, up to 4 m | TF-Luna 8 m if >4 m / >50 Hz required; TFmini-S for longer-range requirement | KEEP architecture, VERIFY optics/calibration |
| Multimedia MCU | ESP32-S3-WROOM-1-N16R8 | Same class/PSRAM variant only after pin/resource audit | KEEP |
| Safety MCU | STM32G0B1RET6 | Same-family equivalent only after I/O/timer/CAN audit | KEEP |
| Main MCU link | Classical CAN/TWAI | UART only as development fallback | KEEP |
| IMU | LSM6DSOX | BMI270 | KEEP |
| Downward cliff | 4× VL53L4CD | Add mechanical feelers if optical blind spots remain | KEEP architecture |
| Horizontal proximity | 4× VL53L4CD | 2× if cost/space pressure, bumper remains | CONDITIONAL |
| Bumper | 4× D2F-L class | D2HW sealed class | KEEP architecture |
| Wheel motor | 25SG-370CA-78-EN class, 12V encoded | Equivalent 25mm encoded gearmotor meeting envelope/performance | CONDITIONAL on final mass |
| Wheel driver | 2× DRV8876 | Equivalent protected H-bridge after current/thermal audit | KEEP |
| Ball caster | POM ball-caster cartridge | Pololu #2692 / custom | CONDITIONAL geometry |
| Battery | 3S2P Molicel M35A class | Trusted equivalent 18650 only after current/energy audit | KEEP architecture |
| Pack protection | BQ7791500 class | Exact family variant after pack/electrical design | CONDITIONAL implementation |
| Charger V1 | Certified 12.6V ~2A CC/CV external | Integrated USB-C PD later | KEEP V1 |
| General bucks | AP62300 class | Equivalent efficiency/thermal parts | KEEP class |
| Safety 3.3V | TPS62162 class | Equivalent isolated safety supply | KEEP class |
| Arm/head protection | TPS25947/eFuse class | Equivalent current-limited protected switch | KEEP class |
| Shoulder | STS3215-C018, 2/arm | Ratio/actuator upgrade only if load/thermal test fails | CONDITIONAL physical test |
| Elbow | HL2915-C001 + ~2:1 belt | STS3215 if torque/thermal margin fails | CONDITIONAL physical test |
| Telescope | N20 encoder + T5×2 | Faster N20 / changed lead after speed test | KEEP architecture |
| Forearm Roll | SC09 | ST3036 if precision/load fails | CONDITIONAL |
| Wrist Pitch | SC09 + ~2:1 | ST3036 / HL2915 if tool load fails | CONDITIONAL |
| Pan | SC09 + 1:1 | ST3036 if jitter/backlash unacceptable | KEEP initial |
| Tilt | SC09 + ~2:1 | ST3036 precision / HL2915 torque | CONDITIONAL on head mass |
| Head Lift | N20 encoder + T5×2 | Faster N20 / stepper only if justified | KEEP architecture |
| Gripper | SC09 adaptive tendon differential | Stronger actuator if grasp tests require | KEEP concept |
| Magnetic Tool | P20/15 12V electromagnet class | Larger magnet / EPM only if requirements justify | CONDITIONAL physical test |
| Tool MCU | STM32C011 class | Similar low-cost MCU with required UART/ADC/PWM/watchdog | KEEP class |
| Tool data | Independent RS-485 per wrist | — | KEEP |
| Microphones | 2× DMM-4026 I2S class | T5848; 4-mic only after testing | KEEP initial |
| Speaker | AS04004PO-R class | AS02404PO if packaging forces smaller unit | CONDITIONAL CAD |
| Audio amp | MAX98357A | Equivalent I2S class-D | KEEP |
