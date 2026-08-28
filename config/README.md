# ZI-E command-center configuration

Copy `.env.example` to `.env`; `scripts/setup.ps1` does this and generates a local
bearer token. `.env` is local-only and must not be committed. The server refuses
non-loopback binding, short bearer tokens, malformed booleans, and invalid ports.

`ZIE_SIMULATOR=false` is the safe default. In that mode, robot telemetry remains
`null` until a real, commissioned semantic adapter supplies confirmed state. Setting
it to `true` labels all robot state as simulated; it does not grant physical authority.

`ZIE_VOICE_ID` is the sole voice selection. ElevenLabs is primary when its key is
configured. `ZIE_VOICE_FALLBACK_COMMAND` receives redacted text on stdin and the voice
selection as `--voice-id VALUE`; commands run without a shell.
