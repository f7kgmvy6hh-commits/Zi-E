from pathlib import Path
import time

from app.server import control_center


def test_fixed_launchers_use_repository_root_and_safe_codex_arguments(monkeypatch):
    paths = {"explorer.exe": r"C:\\Windows\\explorer.exe", "wt.exe": r"C:\\Windows\\wt.exe",
             "codex.exe": r"C:\\Tools\\codex.exe", "hermes.exe": r"C:\\Tools\\hermes.exe"}
    monkeypatch.setattr(control_center.shutil, "which", lambda name: paths.get(name))
    actions = control_center.developer_actions(Path("runtime/zie.log.jsonl"), "hermes")
    root = str(control_center.REPOSITORY_ROOT)
    assert actions["open_repository"].command == (paths["explorer.exe"], root)
    assert actions["open_terminal"].command == (paths["wt.exe"], "-d", root)
    codex = actions["launch_codex"].command
    assert codex == (paths["wt.exe"], "-d", root, paths["codex.exe"], "--cd", root,
                     "--sandbox", "workspace-write", "--ask-for-approval", "never", "--search")
    assert "--yolo" not in codex
    assert actions["launch_hermes"].available is True


def test_drive_deadman_expires_and_mismatched_actuator_stop_is_rejected():
    preview = control_center.WorkspacePreview(True)
    assert preview.drive_request("forward", 20, True)["direction"] == "forward"
    time.sleep(0.55)
    assert preview.drive_status()["request_status"] == "DEADMAN_EXPIRED"
    assert preview.drive_status()["direction"] == "stop"
    preview.commission("head.pan", "enable", 10)
    try:
        preview.commission("head.tilt", "stop", 10)
    except RuntimeError as exc:
        assert "active commissioning actuator is head.pan" in str(exc)
    else:
        raise AssertionError("mismatched actuator stop was accepted")
    assert preview.active_actuator == "head.pan"


def test_hermes_requires_exact_fixed_configuration_and_logs_are_contained(monkeypatch):
    monkeypatch.setattr(control_center.shutil, "which", lambda name: name)
    actions = control_center.developer_actions(Path("../secret.txt"), "hermes --model bypass")
    assert actions["launch_hermes"].available is False
    assert actions["open_logs"].available is False
    assert control_center._contained_log_path(Path("runtime/zie.log.jsonl")) is not None
    assert control_center._contained_log_path(Path("../outside.log")) is None


def test_no_raw_hardware_or_general_shell_routes_exist(tmp_path):
    from fastapi.testclient import TestClient
    from app.server.main import create_app
    from tests.app.test_server import settings
    with TestClient(create_app(settings(tmp_path))) as client:
        paths = {route.path.lower() for route in client.app.routes}
    forbidden = ("terminal", "shell", "gpio", "pwm", "can/transmit", "raw", "flash")
    assert not any(token in path for path in paths for token in forbidden)
