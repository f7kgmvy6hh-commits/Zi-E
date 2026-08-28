# Final Software Integration Audit — 2026-08-28

## Integrated contract

The audited chain is package policy -> registry identity/trust/capability -> exact
profile resolution -> ExtensionHost epoch/context -> subsystem authorization ->
Semantic Robot API -> internal authoritative core -> VirtualRobot state/event. Runtime
coordinates startup, health, reverse revocation, and recovery without becoming a
second authority.

The authoritative end-to-end scenario is covered by the combined deterministic host
test suite: package declaration/verification/readiness; registry registration and
device binding; transactional configuration; profile declaration/validation/
resolution/activation; runtime startup; provider/presentation readiness; SDK activation;
semantic command acceptance and VirtualRobot execution; state/event handling; bounded
provider fallback and optional degradation; revocation; checkpoint; shutdown; stopped
recovery; exact-device re-resolution; fresh profile/epoch/session activation; and
return to running. Assertions compare current registry authorization generation,
profile resolution generation, SDK epoch, runtime generation, configuration
generation, and stale command-session behavior at their owning boundaries.

## Adversarial matrix

| Cross-subsystem attack/failure | Required result |
|---|---|
| quarantine during running; retained pre-update/pre-rollback contexts | runtime fails/degrades by requiredness; every old handle stays stale |
| profile device revoked while service retained; same capability on replacement | active profile clears; no rebinding; retained service denied |
| configuration or registry generation changes while runtime lives | health mismatch fails closed; prior snapshot cannot grant authority |
| provider loses authorization during fallback | skip without call, typed authorization loss, bounded exhaustion |
| selected pack revoked | purge all affected contexts; explicit reactivation required |
| subscriber repeatedly throws | bounded retry/dead-letter; peer subscribers isolated |
| required extension failure plus optional degradation | required failure dominates |
| checkpoint package quarantined/content changed/device differs | recovery rejected; checkpoint grants nothing |
| stale semantic session after restart | retired session rejected; fresh host session required |
| unknown lifecycle/enum/domain | closed-domain rejection without mutation |
| generation exhaustion | typed rejection; no wrap to fresh authority |
| simultaneous failures | bounded provider/event/recovery/watchdog attempts |

## Provider modality policy

LLM, STT, TTS, and Wake have separate exact semantic-capability chains. Priority is a
host/operator configuration with monotonic generation; registration order and provider
output cannot self-promote. Each attempt rechecks exact package/device/category/
capability/lifecycle authority. Temporary failure does not rewrite policy, so the
primary is reconsidered on the next call only if currently authorized. Diagnostics and
attempts are bounded. No credentials, network calls, or Codex development authority
are part of ordinary STT/TTS/LLM/Wake invocation.

## Scope conclusion

No duplicate authority, raw-control SDK path, silent device/profile rebinding,
checkpoint authority, or simulation-to-physical promotion is part of the frozen
contract. Production isolation, persistence, cryptography, network providers,
drivers, CAN/wire protocol, scheduling, and physical commissioning remain explicit
future gates.

