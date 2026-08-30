# ESP32-S3 target boundary

This is a disabled ESP-IDF composition boundary, not a board binding. The generic
ESP32-S3 Presence Runtime build uses `-DZIE_UNVERIFIED_GENERIC_BUILD=ON`; it compiles with
camera, display, audio, TWAI, production run, and all board bindings disabled and
contains no pin values. Its bounded protocol envelope, lifecycle, Host-link/face-mode,
Face Pack metadata, diagnostics limits, fixed safety semantics, and truthful capability
states compile without peripheral component dependencies. Production composition still fails unless the caller supplies
a repository-reviewed profile containing `ZIE_BOARD_PROFILE_VERIFIED=TRUE` and an
active official ESP-IDF environment.

No verified profile exists while received `HW-002` still lacks complete board-specific
verification. Do not add placeholder GPIOs or confuse it with the separately bench-
validated spare HW678 / S3-N16R8 board.
Camera, TFT, I2C, I2S, microSD, USB and TWAI pins remain `UNVERIFIED_BOARD_PIN`; seller
maps are procurement references only. Vendor headers and future physical pins belong
inside concrete drivers below `firmware/drivers/esp32`, never in services, App or HUD.

The future composition must begin with USB logic-only boot and zero peripherals. Flash
and monitor require the operator to select the reviewed board profile and exact device
identity/port; discovery order is forbidden.

The fixed no-flash workflow is:

```powershell
eim run "powershell -ExecutionPolicy Bypass -File .\scripts\build-esp32.ps1"
```

It always cleans, selects `esp32s3`, reconfigures, builds, and runs `idf.py size`. It
accepts no board, port, credential, flash, or arbitrary shell argument. Normal IDF
outputs and the runtime-only build status record are ignored by Git.

## First official build result

ESP-IDF v6.0.2 configured, compiled, linked, generated the bootloader/application
images, and completed `size` from the canonical path containing spaces. Directly
invoking `$IDF_PATH\tools\idf.py` through EIM's Python with a PowerShell argument array
avoids the Windows `idf.py` launcher-association quoting defect; no junction is needed.

The generic footprint was 160,471 bytes total by the IDF size tool; the application
binary was `0x27350` bytes in a `0x100000` software build partition. DIRAM was
47,249/341,760 bytes (13.83%). The separate 16 KiB IRAM row was 100% because ESP-IDF's
ESP32-S3 linker layout fills the dedicated `0x40374000–0x40378000` slice before IRAM
text continues into shared D/IRAM. The map's full `iram0_0_seg` fit assertion passed;
this is region accounting, not an IRAM overflow. Reassess as real drivers are added.

ESP-IDF's upstream Kconfig chooses 2 MB as its default flash-image build setting. Here
that is **BUILD CONFIGURATION ONLY / NOT PHYSICAL EVIDENCE**. It neither describes nor
verifies HW-002. PSRAM remains disabled. The separately verified 16 MB flash / 8 MB OPI
PSRAM of spare HW678 / S3-N16R8 must not be transferred to the HW-002 profile.

The configuration notes about NimBLE boolean defaults, FatFS boolean defaults, and
duplicate Bluetooth rename mappings originate under ESP-IDF v6.0.2 components. Zi-E
defines none of those symbols; configuration finished successfully, so there is no
project-owned Kconfig defect to suppress in this pass.
