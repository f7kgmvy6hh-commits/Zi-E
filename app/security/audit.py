from __future__ import annotations

from datetime import datetime, timezone
import json
from pathlib import Path
from threading import Lock
from typing import Any

from app.security.redaction import redact


class AuditLog:
    def __init__(self, path: Path):
        self.path = path
        self._lock = Lock()

    def write(self, event: str, **fields: Any) -> None:
        record = {"timestamp": datetime.now(timezone.utc).isoformat(), "event": event, **redact(fields)}
        self.path.parent.mkdir(parents=True, exist_ok=True)
        with self._lock, self.path.open("a", encoding="utf-8") as stream:
            stream.write(json.dumps(record, separators=(",", ":"), ensure_ascii=False) + "\n")
