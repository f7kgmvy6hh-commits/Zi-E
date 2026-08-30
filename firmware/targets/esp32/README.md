# ESP32-S3 target boundary

This is a disabled ESP-IDF composition boundary, not a board binding. A generic
architecture smoke build may use `-DZIE_UNVERIFIED_GENERIC_BUILD=ON`; it compiles with
camera, display, audio, TWAI, production run, and all board bindings disabled and
contains no pin values. Production composition still fails unless the caller supplies
a repository-reviewed profile containing `ZIE_BOARD_PROFILE_VERIFIED=TRUE` and an
active official ESP-IDF environment.

No verified profile exists while `HW-002` is ordered. Do not add placeholder GPIOs.
Camera, TFT, I2C, I2S, microSD, USB and TWAI pins remain `UNVERIFIED_BOARD_PIN`; seller
maps are procurement references only. Vendor headers and future physical pins belong
inside concrete drivers below `firmware/drivers/esp32`, never in services, App or HUD.

The future composition must begin with USB logic-only boot and zero peripherals. Flash
and monitor require the operator to select the reviewed board profile and exact device
identity/port; discovery order is forbidden.
