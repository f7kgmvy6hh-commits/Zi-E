from __future__ import annotations

from pathlib import Path
from tempfile import NamedTemporaryFile
from typing import Any
import asyncio
import json
import subprocess
import sys


_WORKER_TIMEOUT_SECONDS = 300
_MAX_WORKER_OUTPUT_BYTES = 64 * 1024


def _failure(error: str) -> dict[str, str | bool]:
    return {"success": False, "transcript": "", "provider": "local", "error": error}


def _validated_worker_result(stdout: bytes) -> dict[str, str | bool]:
    if len(stdout) > _MAX_WORKER_OUTPUT_BYTES:
        return _failure("STT_WORKER_INVALID_OUTPUT")
    try:
        result = json.loads(stdout.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError):
        return _failure("STT_WORKER_INVALID_OUTPUT")
    if not isinstance(result, dict) or type(result.get("success")) is not bool:
        return _failure("STT_WORKER_INVALID_OUTPUT")
    if result.get("provider") != "local" or not isinstance(result.get("transcript"), str):
        return _failure("STT_WORKER_INVALID_OUTPUT")
    if result["success"]:
        if "error" in result:
            return _failure("STT_WORKER_INVALID_OUTPUT")
        return {"success": True, "transcript": result["transcript"], "provider": "local"}
    if not isinstance(result.get("error"), str) or not result["error"]:
        return _failure("STT_WORKER_INVALID_OUTPUT")
    return _failure(result["error"][:500])


class LocalTranscriber:
    def __init__(self, model_name: str = "base", language: str | None = None):
        self.model_name = model_name
        self.language = language or None
        self._model: Any = None

    def status(self) -> str:
        if self._model is not None:
            return "ready"
        try:
            import faster_whisper  # noqa: F401
        except ImportError:
            return "unavailable"
        return "available_not_loaded"

    def _get_model(self):
        if self._model is None:
            from faster_whisper import WhisperModel
            self._model = WhisperModel(self.model_name, device="cpu", compute_type="int8")
        return self._model

    def _transcribe_bytes_in_process(self, audio: bytes, suffix: str = ".wav") -> dict[str, str | bool]:
        if not audio:
            return _failure("audio is empty")
        with NamedTemporaryFile(suffix=suffix, delete=False) as handle:
            path = Path(handle.name)
            handle.write(audio)
        try:
            segments, _ = self._get_model().transcribe(str(path), language=self.language, beam_size=1)
            transcript = " ".join(segment.text.strip() for segment in segments).strip()
            return {"success": True, "transcript": transcript, "provider": "local"}
        except Exception as exc:
            return _failure(str(exc)[:500])
        finally:
            path.unlink(missing_ok=True)

    async def transcribe_isolated(self, audio: bytes, suffix: str = ".wav") -> dict[str, str | bool]:
        if not audio:
            return _failure("audio is empty")
        with NamedTemporaryFile(suffix=suffix, delete=False) as handle:
            path = Path(handle.name)
            handle.write(audio)
        proc = None
        try:
            proc = await asyncio.create_subprocess_exec(
                sys.executable, "-m", "app.voice.stt_worker", str(path), self.model_name,
                stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE,
            )
            stdout, _ = await asyncio.wait_for(
                proc.communicate(), timeout=_WORKER_TIMEOUT_SECONDS
            )
            if proc.returncode == 0:
                return _validated_worker_result(stdout)
            return _failure("STT_WORKER_CRASHED")
        except asyncio.TimeoutError:
            if proc is not None:
                proc.kill()
                await proc.wait()
            return _failure("STT_WORKER_TIMEOUT")
        except asyncio.CancelledError:
            if proc is not None and proc.returncode is None:
                proc.kill()
                await proc.wait()
            raise
        except (OSError, subprocess.SubprocessError):
            return _failure("STT_WORKER_UNAVAILABLE")
        finally:
            path.unlink(missing_ok=True)
