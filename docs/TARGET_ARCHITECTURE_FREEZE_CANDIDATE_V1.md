# Target Architecture Freeze Candidate V1

Status: `FREEZE_CANDIDATE`; software responsibility boundaries only. Hardware
bindings, physical constants, first power, and Phase2B2 are not frozen.

## Ownership and authority

`Zi-E Brain / HostRuntime -> authenticated semantic transport -> ESP32-S3 Presence
Runtime -> bounded semantic controller link -> STM32 Safety / Motion Authority`.

- HostRuntime coordinates Robot API, services, profiles, identity, providers, and
  copied readiness. It has no physical actuator authority.
- ESP32 owns network edge, audio, camera, display/face, local wake/status behavior,
  and a semantic bridge. It cannot override STM32 or invent offline motion.
- STM32 alone owns physical safety enforcement, trajectory/actuator representation,
  hard limits, and the right to reject or stop motion.
- Provider Manager owns independent LLM/STT/TTS/Vision chains. Hermes is an LLM
  gateway, not a robot authority or an implied STT/TTS/Vision implementation.
- XiaoZhi is a pinned engineering donor, never Zi-E's brain, runtime dependency, or
  authority model.

The only motion path is `HostRuntime -> ESP32 semantic bridge -> STM32 -> hardware`.
No public boundary exposes GPIO, PWM, registers, timers, raw servo/motor/current
units, raw CAN transmit, or safety bypass. AI, providers, MCP, plugins, face, camera,
audio, and HUD cannot grant authority.

Every command-capable flow preserves `session_id`, `generation_id`, and `command_id`.
Restarts, reconnects, peer reset, generation replacement, timeout, malformed input,
or link loss retire authority; old authority never reactivates. `ACCEPTED` is not
`EXECUTED` or `COMPLETED`. STOP is idempotent and higher priority than ordinary
motion; the physical kill/E-stop remains higher still.

## Target boundaries

- Host/ESP32: [HOST_ESP32_INTERFACE_V1.md](HOST_ESP32_INTERFACE_V1.md).
- ESP32/STM32: [ESP32_STM32_SEMANTIC_INTERFACE_V1.md](ESP32_STM32_SEMANTIC_INTERFACE_V1.md),
  built on existing Phase2A and Phase2B1 session/lifecycle foundations.
- Providers: [PROVIDER_INTERFACE_V1.md](PROVIDER_INTERFACE_V1.md).
- Presence: [FACE_ENGINE_INTERFACE_V1.md](FACE_ENGINE_INTERFACE_V1.md).
- Offline critical voice: [LOCAL_SAFETY_VOICE_CONTRACT_V1.md](LOCAL_SAFETY_VOICE_CONTRACT_V1.md).
- Existing authority, identity, profile, package, extension, commissioning, and App
  boundaries remain authoritative in `SOFTWARE_ARCHITECTURE_FREEZE_CANDIDATE.md`.

## Explicitly unfrozen

All GPIO/pins, board/camera/touch identity, CAN bitrate/IDs/packing/fragmentation,
heartbeat/deadline/queue values, transceiver/termination/wiring, electrical ratings,
motor/servo/encoder constants, flash partitions, and physical timings remain
evidence-gated. `HW-002` remains ordered and unverified. Phase2B2 remains
`WAITING_FOR_VERIFIED_INPUTS`; first power and physical commissioning are not done.

## Adversarial closure

The candidate was challenged for AI-to-PWM, MCP-to-motor, ESP32-over-STM32,
Host-to-STM32 bypass, stale-session replay, ACCEPTED-as-COMPLETED, offline-face motion,
executable/oversized Face Packs, arbitrary OTA URL, seller-pin promotion, donor-board
inventory promotion, Hermes authority, cross-modality fallback, free-form safety
voice, and online random faces. Each is denied by the boundaries above plus bounded
pack/face/safety tests. Remaining wire authentication, archive parsing/storage budget,
physical timing, and controller-link fault injection stay open gates—not implied proof.
