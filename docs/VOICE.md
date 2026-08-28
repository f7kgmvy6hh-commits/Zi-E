# Voice

`POST /api/voice/speak` accepts authenticated text and streams `audio/mpeg`.
Cloud-bound text is redacted. `ZIE_VOICE_ID` and `ZIE_ELEVENLABS_MODEL` centrally
select the ElevenLabs voice and model; the key never appears in settings or events.

`POST /api/voice/stop` signals cancellation, stopping subsequent chunks when the
generator regains control; it cannot retract client-buffered audio. Muting blocks new
speech. Without ElevenLabs, `ZIE_VOICE_FALLBACK_COMMAND` retains Zi-Nanami command
fallback and receives redacted stdin without shell interpolation.
