"""Offline fallback runner for this repository's plain-assert pytest tests.

Use only when pytest cannot be installed. It supplies the two simple fixtures used by
the suite and does not pretend to implement pytest itself.
"""
from __future__ import annotations

import importlib
import inspect
import os
from pathlib import Path
import tempfile
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))


class MonkeyPatch:
    def delenv(self, key: str, raising: bool = True):
        if raising and key not in os.environ:
            raise KeyError(key)
        os.environ.pop(key, None)


def main() -> int:
    failures = []
    count = 0
    modules = [
        importlib.import_module("tests.app.test_core"),
        importlib.import_module("tests.app.test_robot_voice_integrations"),
        importlib.import_module("tests.app.test_server"),
    ]
    with tempfile.TemporaryDirectory(prefix="zie-tests-") as directory:
        fixtures = {"monkeypatch": MonkeyPatch(), "tmp_path": Path(directory)}
        for module in modules:
            for name, function in inspect.getmembers(module, inspect.isfunction):
                if not name.startswith("test_") or function.__module__ != module.__name__:
                    continue
                count += 1
                try:
                    arguments = {parameter: fixtures[parameter] for parameter in inspect.signature(function).parameters}
                    function(**arguments)
                    print(f"PASS {module.__name__}.{name}")
                except Exception as exc:  # intentionally reports every failure
                    failures.append((module.__name__, name, exc))
                    print(f"FAIL {module.__name__}.{name}: {exc!r}")
    print(f"{count - len(failures)} passed, {len(failures)} failed")
    return bool(failures)


if __name__ == "__main__":
    raise SystemExit(main())
