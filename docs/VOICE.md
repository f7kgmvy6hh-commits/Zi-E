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
