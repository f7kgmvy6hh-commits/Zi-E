# Local Safety Voice Contract V1

Status: fixed-grammar, unauthoritative recognition model implemented and host tested;
audio thresholds, DSP model, wiring, and physical dispatch remain unverified.

The entire critical grammar is `STOP`, `FREEZE`, `EMERGENCY_STOP`. Ordinary words,
including SLEEP and movement phrases, are rejected. Recognition is deterministic and
does not use free-form LLM interpretation.

Target path: `Mic -> ESP32 local recognizer -> local safety semantic observation ->
STM32 safety path`. It does not depend on Internet, Hermes, Provider Manager,
HostRuntime planning, MCP, or cloud availability. Recognition itself grants no robot
authority; STM32 validates and applies local safety policy. Repeated STOP observations
are idempotent. Physical motion kill/E-stop remains higher authority.

No speculative confidence threshold, keyword model, latency deadline, I2S pin, audio
level, or safe-stop time is frozen before verified audio hardware and bench evidence.
