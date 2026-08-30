# First official ESP32-S3 cross-build pass — 2026-08-30

The Product Owner requested Zi-E's first official ESP-IDF v6.0.2 ESP32-S3 cross-build
from checkpoint `d71adb253e1fcf8d1f17110f79ede2af8efee35d`, without commit, push, flash,
port discovery, or inferred HW-002 bindings.

The existing target was retained and expanded into a bounded generic Presence Runtime.
No GPIO, I2S, I2C, TWAI, camera, display, codec, motor, or actuator implementation was
added. A fixed workflow performs only clean, target selection, reconfigure, build, and
size. Control Center keeps software, generic-build, verified-board, physical-target,
and flash states separate. Toolchain Doctor separates the launcher from the framework
version and treats pip as optional for the validated uv-managed App environment.

Windows denied the Codex sandbox account execution access to user-installed EIM and
its Python environment, so the official configure/link result was not promoted to PASS.
The exact continuation command in normal PowerShell is:

```powershell
cd 'C:\Users\mohamad alayan\Desktop\Zi-E'
eim run "powershell -ExecutionPolicy Bypass -File .\scripts\build-esp32.ps1"
```

No hardware was flashed or selected. Software-only validation passed.

Follow-up execution exposed a Windows `idf.py` launcher-association quoting defect: the
canonical path was split before ESP-IDF parsed `-B`. The workflow was corrected to call
the official `$IDF_PATH\tools\idf.py` through EIM's Python with an argument array. HW-002
is now physically received/present but remains unverified; the separately bench-tested
spare HW678 / S3-N16R8 board does not establish a final HW-002 board profile.

The corrected normal-PowerShell run then passed clean configuration, ESP32-S3 GCC 15.2
compile, link, bootloader/application generation, and size under ESP-IDF v6.0.2 from
the canonical space-containing path. No junction, flash, port, or hardware access was
used. The application was `0x27350` bytes; IDF reported 160,471 bytes total and DIRAM
at 13.83%. Its IRAM 100% row describes only the dedicated 16 KiB slice before shared
D/IRAM, not exhaustion of the full linker region. The upstream 2 MB flash default is
recorded solely as generic build configuration and supplies no HW-002 evidence.
