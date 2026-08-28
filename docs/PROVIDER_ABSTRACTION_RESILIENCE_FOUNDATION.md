# Provider Abstraction and Resilience Foundation

## Scope

This host-only C++17 foundation defines vendor-neutral request/response contracts for
LLM, speech-to-text, text-to-speech, and wake providers. It includes deterministic
mock providers and an ordered, bounded provider router. It does not include real
providers, networking, secrets, audio capture/playback, codecs, persistence,
scheduling, UI, transport, CAN, or hardware drivers.

## Authorization boundary

Every provider binding carries a package ID, logical device instance ID, a unique
registry authorization capability, and a semantic routing capability. Each invocation
names its requested semantic capability explicitly; providers of the same kind with a
different semantic capability are skipped and never called. The router invokes a
matching provider only when the
package/device exactly matches an active extension-registry record, the exact binding
registry capability is active, both capability prefixes match the provider kind, and the
manifest category is the corresponding AI/voice provider category.

Authorization is rechecked immediately before every attempted call, so later
quarantine, disable, removal, or capability revocation takes effect without rebuilding
the router. Unique instance capabilities preserve the registry rule that an exact
active capability has only one owner while still allowing an explicitly ordered set
of swappable providers.

## Bounded resilience

The router has fixed maximum provider, attempt, and diagnostic counts. It evaluates matching
providers in registration order, calls each eligible provider at most once per
invocation, catches provider exceptions, rejects empty or wrong-kind successful
responses, and stops when a valid result is returned or the attempt bound is reached.
It never calls the Semantic Robot API, mutates robot state, grants capabilities, or
exposes hardware control. Outcomes retain typed temporary, permanent, malformed,
exception, authorization-loss, and capability-mismatch diagnostics. Successful
fallback retains prior failures; exhausted outcomes retain the final typed cause.
When diagnostic capacity is reached, the oldest record is deterministically removed
and truncation is reported.

The deterministic mock consumes a finite scripted result sequence and exposes its call
count. Tests cover all four kinds, failure, exhaustion, exception isolation, malformed
success responses, recovery, impersonation, missing capabilities, kind mismatch,
invalid input, and lifecycle revocation.

## Deferred work

Timeouts, concurrency, cancellation, rate limits, health scoring, observability events,
provider-specific configuration, authentication, real media buffers, and external
integrations remain future stages. No physical or autonomous-motion safety claim is
made by this foundation.

## Final integration priority policy — 2026-08-28

Provider selection supports host/operator-authored, monotonic priority policies for
each exact semantic capability within each independent LLM, STT, TTS, or Wake
modality. Every listed package must already have a matching registered binding;
duplicates, cross-modality entries, unknown kinds, missing packages, and stale policy
generations fail closed. Provider registration and output cannot change priority.

Invocation reauthorizes every attempt against the registry. A temporary failure
affects only that invocation: it does not mutate policy or permanently lock the router
to a fallback. The next invocation starts at the configured primary and calls it only
if its exact current identity/category/capability/lifecycle authorization remains
valid. Tests are deterministic and contain no network, credentials, or vendor SDK.
