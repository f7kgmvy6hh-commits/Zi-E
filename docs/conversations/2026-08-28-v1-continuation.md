# Zi-E V1 continuation — 2026-08-28

## Verified changes

- Zi-E local server runs loopback-only on `127.0.0.1:8766`; the existing Zi-Nanami server remains on `127.0.0.1:8765`.
- Hermes Agent `v0.20.6` is connected through the installed Windows executable and a real `/api/chat` request returned the expected response.
- `ZIE_SIMULATOR=true` is enabled for safe host-only testing. No physical hardware transport was enabled.
- ElevenLabs voice configuration uses the approved Sarah premade voice and `eleven_flash_v2_5`; a real `/api/voice/speak` request returned audio.
- HUD was redesigned as an original Zi-E cinematic robotics interface with a central cognitive core, truthful disconnected states, live controls, chat input, and WebSocket event support.
- HUD plugins are externally registered through `app/hud/plugins/registry.js` and served by the Zi-E backend under `/hud/`.
- Lifecycle scripts were repaired for this Windows/.NET environment and verified through start, health, stop, and restart cycles.
- The fallback test runner compatibility issue was fixed.

## Verification

- Full local fallback suite: `24 passed, 0 failed`.
- Live health, HUD, registry, simulator command, and E-stop checks passed.
- Restart cycle passed twice.
- `git diff --check` passed; only Windows line-ending notices remain.

## Known limits

- Physical hardware remains disabled; simulator values must remain clearly labeled.
- Hermes smart routing is present in the bridge contract but has not been expanded beyond the existing deterministic route behavior.
- The current HUD plugin registry is a lightweight host registry, not yet a sandboxed executable plugin loader.
- Zapier MCP is authenticated and available, but no Zapier action has been enabled or executed.
