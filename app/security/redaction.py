from __future__ import annotations

import re
from typing import Any

_SECRET_KEYS = re.compile(r"(api[_-]?key|authorization|bearer|password|secret|token)$", re.I)
_TEXT_SECRETS = re.compile(r"(?i)(?:sk-[a-z0-9_-]{16,}|bearer\s+[a-z0-9._~+/-]{12,})")


def redact(value: Any) -> Any:
    if isinstance(value, dict):
        return {key: "[REDACTED]" if _SECRET_KEYS.search(str(key)) else redact(item) for key, item in value.items()}
    if isinstance(value, list):
        return [redact(item) for item in value]
    if isinstance(value, tuple):
        return tuple(redact(item) for item in value)
    if isinstance(value, str):
        return redact_text(value)
    return value


def redact_text(text: str) -> str:
    return _TEXT_SECRETS.sub("[REDACTED]", text)
