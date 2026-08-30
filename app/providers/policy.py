from __future__ import annotations

from dataclasses import asdict, dataclass
from enum import Enum
from typing import Callable


class Modality(str, Enum):
    LLM = "llm"
    STT = "stt"
    TTS = "tts"
    WAKE = "wake"
    VISION = "vision"


@dataclass(frozen=True)
class ProviderStatus:
    provider_id: str
    modality: Modality
    capability: str
    priority: int
    configured: bool
    authorization: str
    health: str
    scope: str = "app-local-non-robot"
    selection_reason: str = "UNAVAILABLE"
    latency_ms: int | None = None
    last_failure: str | None = None
    primary_eligible: bool = False

    def public(self) -> dict[str, object]:
        value = asdict(self)
        value["modality"] = self.modality.value
        eligible = self.configured and self.health in {"configured", "healthy", "ready"}
        value["primary_eligible"] = eligible
        value["selection_reason"] = "PRIMARY" if eligible and self.priority == 1 else (
            "FALLBACK_READY" if eligible else "UNAVAILABLE"
        )
        return value


@dataclass(frozen=True)
class ProviderAttempt:
    provider_id: str
    outcome: str


@dataclass(frozen=True)
class ProviderResult:
    success: bool
    provider_id: str | None
    value: object | None
    failure: str | None
    attempts: tuple[ProviderAttempt, ...]


@dataclass
class ProviderBinding:
    status: ProviderStatus
    invoke: Callable[[object], object]
    recheck: Callable[[], tuple[bool, str]]


class ExactModalityChain:
    """Bounded non-sticky testable routing; it is not HostRuntime authority."""

    def __init__(self, modality: Modality, capability: str, bindings: list[ProviderBinding]):
        if not capability or len(bindings) > 8:
            raise ValueError("invalid provider chain")
        self.modality = modality
        self.capability = capability
        self._bindings = tuple(sorted(bindings, key=lambda item: item.status.priority))
        if len({item.status.priority for item in self._bindings}) != len(self._bindings):
            raise ValueError("provider priorities must be unique")
        for item in self._bindings:
            if item.status.modality is not modality or item.status.capability != capability:
                raise ValueError("provider modality/capability mismatch")

    def invoke(self, request: object) -> ProviderResult:
        attempts: list[ProviderAttempt] = []
        for binding in self._bindings:
            status = binding.status
            if not status.configured:
                attempts.append(ProviderAttempt(status.provider_id, "not_configured"))
                continue
            authorized, health = binding.recheck()
            if not authorized:
                attempts.append(ProviderAttempt(status.provider_id, "unauthorized"))
                continue
            if health != "healthy":
                attempts.append(ProviderAttempt(status.provider_id, health))
                continue
            try:
                value = binding.invoke(request)
            except Exception:
                attempts.append(ProviderAttempt(status.provider_id, "provider_failure"))
                continue
            return ProviderResult(True, status.provider_id, value, None, tuple(attempts))
        return ProviderResult(False, None, None, "fallback_exhausted", tuple(attempts))


def app_provider_status(settings, stt_status: str) -> dict[str, list[dict[str, object]]]:
    llm_provider = settings.hermes_main_model.partition("/")[0]
    openai_llm = bool(settings.hermes_command) and llm_provider.startswith("openai")
    llm_entries = [
        ProviderStatus("openai-hermes", Modality.LLM, "provider.invoke.llm", 1,
                       openai_llm, "not_verified", "configured" if openai_llm else "not_configured"),
    ]
    if settings.hermes_command and not openai_llm:
        llm_entries.append(
            ProviderStatus(llm_provider or "configured-hermes", Modality.LLM,
                           "provider.invoke.llm", 2, True, "not_verified", "configured")
        )
    entries = {
        Modality.LLM: llm_entries,
        Modality.STT: [
            ProviderStatus("openai-stt", Modality.STT, "provider.invoke.stt", 1,
                           False, "not_verified", "not_configured"),
            ProviderStatus("local-whisper", Modality.STT, "provider.invoke.stt", 2,
                           stt_status != "unavailable", "not_verified", stt_status),
        ],
        Modality.TTS: [
            ProviderStatus("openai-tts", Modality.TTS, "provider.invoke.tts", 1,
                           False, "not_verified", "not_configured"),
            ProviderStatus("elevenlabs", Modality.TTS, "provider.invoke.tts", 2,
                           bool(settings.elevenlabs_api_key), "not_verified",
                           "configured" if settings.elevenlabs_api_key else "not_configured"),
            ProviderStatus("zi-nanami-command", Modality.TTS, "provider.invoke.tts", 3,
                           bool(settings.voice_fallback_command), "not_verified",
                           "configured" if settings.voice_fallback_command else "not_configured"),
        ],
        Modality.WAKE: [
            ProviderStatus("wake-unconfigured", Modality.WAKE, "provider.invoke.wake", 1,
                           False, "not_verified", "not_configured"),
        ],
        Modality.VISION: [
            ProviderStatus("vision-unconfigured", Modality.VISION, "provider.invoke.vision", 1,
                           False, "not_verified", "not_configured"),
        ],
    }
    return {modality.value: [entry.public() for entry in values] for modality, values in entries.items()}
