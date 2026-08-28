from __future__ import annotations

from pathlib import Path
from tempfile import NamedTemporaryFile
from typing import Any


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

    def transcribe_bytes(self, audio: bytes, suffix: str = ".wav") -> dict[str, str | bool]:
        if not audio:
            return {"success": False, "transcript": "", "provider": "local", "error": "audio is empty"}
        with NamedTemporaryFile(suffix=suffix, delete=False) as handle:
            path = Path(handle.name)
            handle.write(audio)
        try:
            segments, _ = self._get_model().transcribe(str(path), language=self.language, beam_size=1)
            transcript = " ".join(segment.text.strip() for segment in segments).strip()
            return {"success": True, "transcript": transcript, "provider": "local"}
        except Exception as exc:
            return {"success": False, "transcript": "", "provider": "local", "error": str(exc)}
        finally:
            path.unlink(missing_ok=True)
