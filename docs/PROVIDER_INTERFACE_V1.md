# Provider Interface V1

Status: contract refinement of `PROVIDER_ABSTRACTION_RESILIENCE_FOUNDATION.md`.

Provider Manager owns independent LLM, STT, TTS, and Vision modality chains. Wake is
an ESP32-local capability, not an LLM provider. Implementations conceptually expose
`capabilities()`, `health()`, `execute()`, `stream()`, and `cancel()` where the
modality supports them. Exact-capability authorization is rechecked per call.

Status records support `active_provider`, `selection_reason`, `fallback_reason`,
`latency`, typed `failure_type`, `health_state`, `primary_eligible`, `last_success`,
and `last_failure`. Fallback is policy, not a provider. A temporary failure affects
the bounded invocation only; the primary is reconsidered on the next call when
eligible. No cross-modality substitution or capability widening is allowed.

Hermes is the configured LLM gateway and may route an operator-configured LLM chain;
it is not represented as STT, TTS, or Vision without a concrete separately authorized
adapter. Model names and routing remain configuration-owned. Local Whisper/Piper-like
implementations are candidates, not installed facts.

Provider output is data. It cannot grant sessions/generations, call raw hardware,
bypass Robot API, or acquire physical authority. AI intent still traverses HostRuntime,
ESP32 semantic transport, STM32 validation, and confirmed physical feedback.
