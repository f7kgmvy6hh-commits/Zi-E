# STM32 target boundary

This disabled CMake boundary reserves future STM32CubeCLT composition without choosing
an MCU, package, PCB, clock tree, timer/PWM/ADC/encoder allocation, GPIO/CAN pins,
interrupt priorities or safe-stop timing. The current STM32 candidate is design input,
not received evidence.

Configuration requires a reviewed profile containing `ZIE_BOARD_PROFILE_VERIFIED=TRUE`
and the STM32CubeCLT-managed GNU Arm compiler, then still stops until a concrete
composition root is deliberately implemented and reviewed. Vendor headers remain in
`firmware/drivers/stm32`. No App/plugin/raw-control path is introduced.

