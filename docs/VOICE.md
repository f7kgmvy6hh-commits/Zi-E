# Voice

`POST /api/voice/speak` accepts authenticated text and streams `audio/mpeg`.
Cloud-bound text is redacted. `ZIE_VOICE_ID` and `ZIE_ELEVENLABS_MODEL` centrally
select the ElevenLabs voice and model; the key never appears in settings or events.

`POST /api/voice/stop` signals cancellation, stopping subsequent chunks when the
generator regains control; it cannot retract client-buffered audio. Muting blocks new
speech. Without ElevenLabs, `ZIE_VOICE_FALLBACK_COMMAND` retains Zi-Nanami command
fallback and receives redacted stdin without shell interpolation.

LLM, STT, TTS, and Wake are separate authoritative modality chains. The current App
adapters do not own priority: when an authorized OpenAI-capable STT or TTS adapter is
configured through HostRuntime it is preferred, with bounded typed fallback. Local
Whisper is presently an isolated STT fallback implementation; ElevenLabs and
Zi-Nanami are presently TTS adapters. No Codex development authority is required for
ordinary transcription or speech synthesis.

Local Whisper runs only in `app.voice.stt_worker`, with a bounded parent wait and a
strict JSON result contract. Native crash, timeout, launch failure, and malformed
output fail closed without taking down the FastAPI process.

`GET /api/providers` reports four independent ordered chains with exact semantic
capabilities. `GET /api/voice/status` reports STT, TTS, and Wake status without keys,
tokens, or provider output. Configuration is not authorization: until HostRuntime
provides live authority, App entries report `authorization: not_verified`.

The currently executable Hermes, ElevenLabs, Zi-Nanami, and local-Whisper adapters are
explicitly labeled `scope: app-local-non-robot`. They may provide conversational
features but grant no robot/profile/plugin/configuration/pack capability. Existing TTS
tries its configured cloud adapter anew on each request and uses the configured local
fallback only on failure; this is a non-authoritative App-local path, not the frozen
HostRuntime provider router. A future privileged integration must perform live
HostRuntime authorization/health checks before invocation.

Deterministic contract tests prove bounded exact-modality fallback and that a
temporarily unhealthy primary is retried on the next request after health/auth
recheck; fallback never rewrites priority. No live OpenAI STT/TTS call or credential
was added. Local Whisper crash, timeout, cancellation, malformed/oversized output,
provider impersonation, empty input, and oversized HTTP input fail closed.
