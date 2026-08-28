from __future__ import annotations

from dataclasses import dataclass
import ipaddress
import os
from pathlib import Path
from typing import Mapping


def _boolean(value: str, name: str) -> bool:
    lowered = value.strip().lower()
    if lowered in {"1", "true", "yes", "on"}:
        return True
    if lowered in {"0", "false", "no", "off"}:
        return False
    raise ValueError(f"{name} must be true or false")


@dataclass(frozen=True)
class Settings:
    host: str
    port: int
    auth_token: str
    simulator: bool
    log_path: Path
    hermes_command: str | None
    hermes_session: str
    elevenlabs_api_key: str | None
    voice_id: str
    voice_fallback_command: str | None
    hermes_main_model: str = "openai-codex/gpt-5.6-sol"
    elevenlabs_model: str = "eleven_multilingual_v2"


def load_settings(source: Mapping[str, str] | None = None) -> Settings:
    values = os.environ if source is None else source
    host = values.get("ZIE_HOST", "127.0.0.1")
    try:
        address = ipaddress.ip_address(host)
    except ValueError as exc:
        raise ValueError("ZIE_HOST must be a loopback IP address") from exc
    if not address.is_loopback:
        raise ValueError("ZI-E server is loopback-only")
    token = values.get("ZIE_AUTH_TOKEN", "")
    if len(token) < 24:
        raise ValueError("ZIE_AUTH_TOKEN must be at least 24 characters")
    port = int(values.get("ZIE_PORT", "8766"))
    if not 1 <= port <= 65535:
        raise ValueError("ZIE_PORT is outside 1..65535")
    return Settings(
        host=host,
        port=port,
        auth_token=token,
        simulator=_boolean(values.get("ZIE_SIMULATOR", "false"), "ZIE_SIMULATOR"),
        log_path=Path(values.get("ZIE_LOG_PATH", "runtime/zie.log.jsonl")),
        hermes_command=values.get("ZIE_HERMES_COMMAND") or None,
        hermes_session=values.get("ZIE_HERMES_SESSION", "zie-command-center"),
        elevenlabs_api_key=values.get("ELEVENLABS_API_KEY") or None,
        voice_id=values.get("ZIE_VOICE_ID", "zie-default"),
        voice_fallback_command=values.get("ZIE_VOICE_FALLBACK_COMMAND") or None,
        hermes_main_model=values.get("ZIE_HERMES_MAIN_MODEL", "openai-codex/gpt-5.6-sol"),
        elevenlabs_model=values.get("ZIE_ELEVENLABS_MODEL", "eleven_multilingual_v2"),
    )
