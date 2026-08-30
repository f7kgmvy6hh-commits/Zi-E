from pathlib import Path
from html.parser import HTMLParser
import json
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


def test_esp32_development_status_separates_build_hardware_and_flash(monkeypatch, tmp_path):
    monkeypatch.setattr(control_center, "ESP32_BUILD_STATUS", tmp_path / "not-run.json")
    status = control_center.esp32_development_status()
    assert status["toolchain"] == "READY"
    assert status["generic_cross_build"] == "PASS"
    assert status["physical_target"] == "HW_002_NOT_ARRIVED_UNVERIFIED"
    assert status["verified_board"] == "BLOCKED_HW_002_NOT_ARRIVED"
    assert status["flash"] == "NOT_AUTHORIZED"
    assert status["local_build_record"] == "NOT_PRESENT"


def test_repository_status_reuses_only_bounded_repository_metadata(monkeypatch):
    calls = []
    now = [100.0]

    def fake_git(*arguments):
        calls.append(arguments)
        return {
            ("status", "--short", "--branch"): "## refactor/modular-hardware-architecture",
            ("rev-parse", "HEAD"): "3243dd78cf5425eb92c0a88cc5dd4b4f58d4bbbe",
            ("log", "-5", "--pretty=format:%h %s"): "3243dd7 checkpoint",
            ("diff", "--stat"): "",
        }[arguments]

    monkeypatch.setattr(control_center, "_git", fake_git)
    monkeypatch.setattr(control_center.time, "monotonic", lambda: now[0])
    control_center.clear_repository_status_cache()
    first = control_center.repository_status()
    first["branch"] = "mutated-by-caller"
    assert control_center.repository_status()["branch"] == "refactor/modular-hardware-architecture"
    assert len(calls) == 4
    now[0] += control_center.REPOSITORY_STATUS_TTL_SECONDS + 0.01
    assert control_center.repository_status()["available"] is True
    assert len(calls) == 8
    control_center.clear_repository_status_cache()


def test_repository_status_git_failure_cannot_look_ready(monkeypatch):
    monkeypatch.setattr(control_center, "_git", lambda *_arguments: None)
    control_center.clear_repository_status_cache()
    status = control_center.repository_status()
    assert status["available"] is False
    assert status["working_tree"] == "unavailable"
    assert status["head"] is None and status["branch"] is None
    control_center.clear_repository_status_cache()


def test_git_safe_directory_uses_cross_platform_forward_slashes(monkeypatch):
    captured = []

    class Result:
        returncode = 0
        stdout = "ok"

    def fake_run(arguments, **_kwargs):
        captured.append(arguments)
        return Result()

    monkeypatch.setattr(control_center.subprocess, "run", fake_run)
    assert control_center._git("rev-parse", "HEAD") == "ok"
    assert captured[0][2] == f"safe.directory={control_center.REPOSITORY_ROOT.as_posix()}"


def test_malformed_project_evidence_fails_closed(monkeypatch, tmp_path):
    bad = tmp_path / "evidence.json"
    bad.write_text(json.dumps({"schema": "zie.project-evidence.v1", "software": {
        "esp32_flash": "AUTHORIZED"}, "hardware": []}), encoding="utf-8")
    monkeypatch.setattr(control_center, "PROJECT_EVIDENCE", bad)
    control_center.clear_project_evidence_cache()
    evidence = control_center.project_evidence()
    assert evidence["evidence_status"] == "INVALID_UNAVAILABLE"
    assert evidence["software"]["esp32_generic_cross_build"] == "NOT_RUN"
    assert evidence["software"]["esp32_flash"] == "NOT_AUTHORIZED"
    assert evidence["hardware"] == []
    control_center.clear_project_evidence_cache()


def test_project_evidence_rejects_unknown_stm32_enum(monkeypatch, tmp_path):
    source = json.loads(control_center.PROJECT_EVIDENCE.read_text(encoding="utf-8"))
    source["software"]["stm32_development"] = "READY_ENOUGH"
    path = tmp_path / "invalid-stm32.json"
    path.write_text(json.dumps(source), encoding="utf-8")
    monkeypatch.setattr(control_center, "PROJECT_EVIDENCE", path)
    control_center.clear_project_evidence_cache()
    assert control_center.project_evidence()["evidence_status"] == "INVALID_UNAVAILABLE"
    control_center.clear_project_evidence_cache()


def test_project_evidence_cache_refreshes_when_file_signature_changes(monkeypatch, tmp_path):
    evidence_path = tmp_path / "evidence.json"
    source = json.loads(control_center.PROJECT_EVIDENCE.read_text(encoding="utf-8"))
    evidence_path.write_text(json.dumps(source), encoding="utf-8")
    monkeypatch.setattr(control_center, "PROJECT_EVIDENCE", evidence_path)
    control_center.clear_project_evidence_cache()
    assert control_center.project_evidence()["evidence_status"] == "VALIDATED_REPOSITORY_EVIDENCE"
    source["hardware"][0]["physical_state"] = "PRESENT_BUT_UNVERIFIED"
    evidence_path.write_text(json.dumps(source, indent=2), encoding="utf-8")
    assert control_center.project_evidence()["evidence_status"] == "INVALID_UNAVAILABLE"
    control_center.clear_project_evidence_cache()


def test_untrusted_local_build_record_cannot_promote_repository_truth(monkeypatch, tmp_path):
    report = tmp_path / "esp32.json"
    report.write_text(json.dumps({"schema": "zie.esp32-generic-build.v1", "target": "esp32s3",
        "profile": "GENERIC_UNVERIFIED_ESP32S3", "result": "READY",
        "flash": "NOT_AUTHORIZED", "idf_version": "spoofed"}), encoding="utf-8")
    monkeypatch.setattr(control_center, "ESP32_BUILD_STATUS", report)
    status = control_center.esp32_development_status()
    assert status["generic_cross_build"] == "PASS"
    assert status["local_build_record"] == "INVALID_IGNORED"
    assert status["physical_target"] == "HW_002_NOT_ARRIVED_UNVERIFIED"


def test_hardware_evidence_is_bounded_and_never_grants_authority():
    control_center.clear_project_evidence_cache()
    evidence = control_center.project_evidence()
    by_id = {item["id"]: item for item in evidence["hardware"]}
    assert evidence["evidence_status"] == "VALIDATED_REPOSITORY_EVIDENCE"
    assert by_id["HW-002"]["physical_state"] == "NOT_ARRIVED"
    assert by_id["SPARE-HW678"]["integration_state"] == "NOT_HW_002"
    assert by_id["HW-006"]["evidence_state"] == "INDEPENDENT_BENCH_PASS"
    assert by_id["HW-007"]["integration_state"] == "QUARANTINED"
    assert by_id["HW-008"]["evidence_state"] == "INDEPENDENT_BENCH_PASS"
    assert by_id["HW-008"]["functional_state"] == "DI_RGB_CHANNELS_3V3_330OHM_PASS"
    assert by_id["HW-008"]["integration_state"] == "NOT_COMMISSIONED_FINAL_5V_NOT_VERIFIED"
    assert "TEMPORARY_GPIO7_NOT_PRODUCTION" in by_id["HW-008"]["restrictions"]
    assert "FINAL_5V_OPERATION_NOT_VERIFIED" in by_id["HW-008"]["restrictions"]
    assert all("authority" not in item for item in evidence["hardware"])


def test_hardware_evidence_rejects_any_promoted_item_state(monkeypatch, tmp_path):
    source = json.loads(control_center.PROJECT_EVIDENCE.read_text(encoding="utf-8"))
    source["hardware"][0]["functional_state"] = "COMMISSIONED"
    path = tmp_path / "promoted.json"
    path.write_text(json.dumps(source), encoding="utf-8")
    monkeypatch.setattr(control_center, "PROJECT_EVIDENCE", path)
    control_center.clear_project_evidence_cache()
    assert control_center.project_evidence()["evidence_status"] == "INVALID_UNAVAILABLE"
    control_center.clear_project_evidence_cache()


def test_hardware_evidence_rejects_removed_safety_restriction(monkeypatch, tmp_path):
    source = json.loads(control_center.PROJECT_EVIDENCE.read_text(encoding="utf-8"))
    by_id = {item["id"]: item for item in source["hardware"]}
    by_id["HW-007"]["restrictions"].remove("DO_NOT_POWER_IN_THIS_PASS")
    path = tmp_path / "missing-restriction.json"
    path.write_text(json.dumps(source), encoding="utf-8")
    monkeypatch.setattr(control_center, "PROJECT_EVIDENCE", path)
    assert control_center.project_evidence()["evidence_status"] == "INVALID_UNAVAILABLE"
    control_center.clear_project_evidence_cache()


def test_hud_uses_one_bounded_control_center_refresh_path_and_memory_only_token():
    html = (control_center.REPOSITORY_ROOT / "app" / "hud" / "index.html").read_text(encoding="utf-8")
    enhancement = (control_center.REPOSITORY_ROOT / "app" / "hud" / "cockpit.js").read_text(encoding="utf-8")
    assert html.count("api('/api/control-center'") == 1
    assert "/api/control-center" not in enhancement
    assert "fetch(" not in enhancement and "setInterval(" not in enhancement
    assert "sessionStorage" not in html and "localStorage" not in html
    assert "refreshController?.abort()" in html
    assert "generation!==refreshGeneration" in html
    assert "refreshGeneration++;refreshController?.abort();refreshController=null" in html
    assert "stale data cleared" in html and "staleIds.forEach" in html
    assert "if(!document.hidden)refresh()" in html
    assert "while(list.children.length>100)" in html


def test_hud_static_ids_are_unique_and_accessibility_boundaries_exist():
    class IdParser(HTMLParser):
        def __init__(self):
            super().__init__(); self.ids = []

        def handle_starttag(self, _tag, attrs):
            values = dict(attrs)
            if "id" in values:
                self.ids.append(values["id"])

    html = (control_center.REPOSITORY_ROOT / "app" / "hud" / "index.html").read_text(encoding="utf-8")
    css = (control_center.REPOSITORY_ROOT / "app" / "hud" / "cockpit.css").read_text(encoding="utf-8")
    parser = IdParser(); parser.feed(html)
    assert len(parser.ids) == len(set(parser.ids))
    assert 'for="token"' in html and 'aria-label="Control Center workspaces"' in html
    assert 'aria-live="polite"' in html
    assert "prefers-reduced-motion:reduce" in css
