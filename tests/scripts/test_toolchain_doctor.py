import json
from pathlib import Path
import subprocess


ROOT = Path(__file__).resolve().parents[2]


def test_toolchain_doctor_is_bounded_json_and_reports_hardware_gates():
    result = subprocess.run(
        ["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
         str(ROOT / "scripts" / "toolchain-doctor.ps1"), "-Json"],
        cwd=ROOT, capture_output=True, text=True, timeout=30, check=True,
    )
    report = json.loads(result.stdout)
    assert report["schema"] == "zie.toolchain-doctor.v1"
    assert report["overall"] in {"READY", "INCOMPLETE"}
    rows = {(row["area"], row["tool"]): row for row in report["tools"]}
    for key in (("HOST", "Python"), ("HOST", "CMake"), ("ESP32", "idf.py launcher"),
                ("ESP32", "ESP32-S3 compiler"), ("STM32", "arm-none-eabi-gcc"),
                ("STM32", "STM32CubeProgrammer CLI")):
        assert key in rows
    assert rows[("ESP32", "compatible physical target")]["status"] == "HARDWARE_REQUIRED"
    assert rows[("STM32", "ST-LINK probe and target")]["status"] == "HARDWARE_REQUIRED"
    assert rows[("HOST", "project pip")]["status"] in {"READY", "OPTIONAL"}
    framework = rows[("ESP32", "ESP-IDF")]
    if framework["status"] == "READY":
        assert framework["version"].startswith("v") or framework["version"][0].isdigit()
        assert framework["version"] != "v1.0.3"
    assert all(row["status"] in {"READY", "MISSING", "OPTIONAL",
        "MANUAL_VENDOR_INSTALL_REQUIRED", "HARDWARE_REQUIRED"} for row in report["tools"])


def test_firmware_workflow_refuses_target_actions_without_verified_identity():
    script = str(ROOT / "scripts" / "firmware-workflow.ps1")
    for action in ("esp32-build", "esp32-flash", "esp32-monitor", "stm32-configure",
                   "stm32-build", "stm32-program"):
        result = subprocess.run(
            ["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", script,
             "-Action", action], cwd=ROOT, capture_output=True, text=True, timeout=15,
        )
        assert result.returncode != 0
        assert "Explicit -BoardProfile is required" in (result.stdout + result.stderr)


def test_disabled_target_boundaries_contain_no_pin_or_bus_values():
    combined = "\n".join((ROOT / path).read_text(encoding="utf-8") for path in (
        "firmware/targets/esp32/CMakeLists.txt", "firmware/targets/esp32/README.md",
        "firmware/targets/stm32/CMakeLists.txt", "firmware/targets/stm32/README.md",
    ))
    assert "BOARD_BINDING_DISABLED" in combined
    assert "ZIE_BOARD_PROFILE_VERIFIED=TRUE" in combined
    forbidden = ("CAN_BITRATE", "TWAI_TX_GPIO", "TWAI_RX_GPIO", "FDCAN_TX_PIN",
                 "FDCAN_RX_PIN", "HEARTBEAT_HZ", "SAFE_STOP_MS")
    assert all(value not in combined for value in forbidden)


def test_generic_build_workflow_is_fixed_no_flash_and_bounded():
    script = (ROOT / "scripts" / "build-esp32.ps1").read_text(encoding="utf-8")
    assert "GENERIC_UNVERIFIED_ESP32S3" in script
    assert "set-target', 'esp32s3" in script
    assert "fullclean" in script and "reconfigure" in script and "build" in script and "size" in script
    assert "param()" in script
    assert "IDF_PATH 'tools\\idf.py'" in script
    assert "& idf.py" not in script
    forbidden = ("'flash'", "monitor", "COM", "Get-CimInstance", "SSID", "password")
    assert all(value not in script for value in forbidden)
