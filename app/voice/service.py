from __future__ import annotations

from dataclasses import dataclass
import inspect
from threading import Event, Lock
from typing import Callable

from app.security.redaction import redact_text


@dataclass(frozen=True)
class VoiceResult:
    result: str
    provider: str | None = None


class VoiceService:
    def __init__(self, voice_id: str, cloud_synth: Callable | None, fallback: Callable | None,
                 model_id: str = "eleven_multilingual_v2"):
        self.voice_id = voice_id
        self.model_id = model_id
        self._cloud_synth = cloud_synth
        self._fallback = fallback
        self._muted = False
        self._active = False
        self._cancelled: Event | None = None
        self._lock = Lock()

    def speak_chunks(self, text: str):
        if self._muted:
            return iter(())
        synth = self._cloud_synth or self._fallback
        if synth is None:
            return iter(())
        safe_text = redact_text(text)
        cancelled = Event()
        with self._lock:
            if self._cancelled:
                self._cancelled.set()
            self._cancelled = cancelled
            self._active = True

        def generate():
            try:
                # Retain compatibility with small in-process/test adapters from V1.
                try:
                    if len(inspect.signature(synth).parameters) >= 4:
                        output = synth(safe_text, self.voice_id, self.model_id, cancelled)
                    else:
                        output = synth(safe_text, self.voice_id)
                    chunks = (output,) if isinstance(output, bytes) else output
                    for chunk in chunks:
                        if cancelled.is_set():
                            break
                        if chunk:
                            yield chunk
                except Exception:
                    if synth is not self._cloud_synth or self._fallback is None or cancelled.is_set():
                        raise
                    fallback = self._fallback
                    if len(inspect.signature(fallback).parameters) >= 4:
                        output = fallback(safe_text, self.voice_id, self.model_id, cancelled)
                    else:
                        output = fallback(safe_text, self.voice_id)
                    chunks = (output,) if isinstance(output, bytes) else output
                    for chunk in chunks:
                        if cancelled.is_set():
                            break
                        if chunk:
                            yield chunk
            finally:
                with self._lock:
                    if self._cancelled is cancelled:
                        self._active = False
                        self._cancelled = None
        return generate()

    def speak(self, text: str) -> VoiceResult:
        if self._muted:
            return VoiceResult("muted")
        safe_text = redact_text(text)
        self._active = True
        try:
            if self._cloud_synth is not None:
                list(self.speak_chunks(safe_text))
                return VoiceResult("accepted", "elevenlabs")
            if self._fallback is not None:
                list(self.speak_chunks(safe_text))
                return VoiceResult("accepted", "zi-nanami-command")
            return VoiceResult("unavailable")
        finally:
            self._active = False

    def mute(self, muted: bool) -> VoiceResult:
        self._muted = muted
        if muted:
            self._active = False
        return VoiceResult("muted" if muted else "unmuted")

    def stop(self) -> VoiceResult:
        with self._lock:
            if self._cancelled:
                self._cancelled.set()
        self._active = False
        return VoiceResult("stopped")
