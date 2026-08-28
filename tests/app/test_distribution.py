from pathlib import Path


ROOT = Path(__file__).parents[2]


def test_exact_lifecycle_scripts_exist_and_compatibility_wrappers_delegate():
    for name in ("setup_zi-e.ps1", "start_zi-e.ps1", "stop_zi-e.ps1", "health_zi-e.ps1"):
        assert (ROOT / "scripts" / name).is_file()
    assert "start_zi-e.ps1" in (ROOT / "scripts" / "start.ps1").read_text(encoding="utf-8")
    start = (ROOT / "scripts" / "start_zi-e.ps1").read_text(encoding="utf-8")
    stop = (ROOT / "scripts" / "stop_zi-e.ps1").read_text(encoding="utf-8")
    assert "ZIE_CAMOFOX_ENABLED" in start and "camofox" in start.lower()
    assert "health_zi-e.ps1" in start and "Start-Process" in start
    assert "owned-processes.json" in start and "owned-processes.json" in stop
    assert "CommandLine" in stop and "Stop-Process" in stop


def test_example_yaml_and_requested_operator_docs_exist():
    for name in ("default.example.yaml", "robot.example.yaml"):
        text = (ROOT / "config" / name).read_text(encoding="utf-8")
        assert "example" in text.lower() or "simulator" in text.lower()
    for name in (
        "ARCHITECTURE.md", "SETUP.md", "ROBOT.md", "VOICE.md", "SECURITY.md",
        "RECOVERY.md", "TROUBLESHOOTING.md",
    ):
        text = (ROOT / "docs" / name).read_text(encoding="utf-8")
        assert text.startswith("# ") and len(text) > 200


def test_hermes_smart_routing_placeholder_is_documented_truthfully():
    architecture = (ROOT / "docs" / "ARCHITECTURE.md").read_text(encoding="utf-8")
    assert "smart_model_routing" in architecture
    assert "not consumed" in architecture.lower()


def test_pytest_cache_and_known_upstream_warning_are_handled():
    config = (ROOT / "pyproject.toml").read_text(encoding="utf-8")
    assert "cache_dir = \"runtime/pytest-cache\"" in config
    assert "Using `httpx` with `starlette.testclient` is deprecated" in config
