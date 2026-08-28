# Current-HEAD App/STT reconciliation — 2026-08-28

## Request and repository authority

The Product Owner directed work to continue from current HEAD `007c40d`, preserving
all intentional App/STT WIP. The six commits after `da5143b` were inspected.
`902bd58` already froze the integrated software foundation and provider architecture;
later commits added the command center, Hermes/HUD integration, voice, and local STT.

## Decisions

- Do not redo or duplicate the frozen C++ provider router.
- The current Python App remains a simulator/operator prototype. It has no real-target
  authority until it consumes HostRuntime/public semantic services.
- Ordinary Hermes requests use the configured LLM primary. Prompt complexity cannot
  alter authoritative provider priority; explicit `/model` remains user-directed.
- Local Whisper remains an isolated STT fallback implementation. Its worker protocol
  is strict, typed, bounded, and cannot impersonate another provider.
- OpenAI-capable primary preference remains separate for LLM, STT, TTS, and Wake.
  Ordinary STT/TTS requires no Codex development authority.
- Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`.

## Validation

Final commands and results are recorded in `CURRENT_STATE.md`. No commit, push,
physical transport, hardware implementation, or commissioning action was performed.
