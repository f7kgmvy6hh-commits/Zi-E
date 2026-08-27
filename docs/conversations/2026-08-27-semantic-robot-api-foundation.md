# 2026-08-27 — Semantic Robot API foundation

Starting from clean `2859f03`, the user requested a bounded configuration repair and
then a coherent semantic Robot API command/event/state foundation. Bounded integers now
require explicit inclusive signed 64-bit limits and exact base-10 parsing; booleans are
canonical lowercase `true`/`false`.

The semantic API adds typed high-level intents authorized against active registry
package/device providers and fixed active capabilities. It separates immutable events
from commands and generation-ordered read-only state from both. Protected safety and
raw control paths are rejected/absent. Tests cover lifecycle/capability denial,
impersonation, stale sequence/state, event isolation, unknown domains, and safety/raw
bypass attempts.

No transport, networking, IPC, CAN/framing, parser, persistence, driver, scheduler, UI,
or AI provider was added. Phase 2B2 remained `WAITING_FOR_VERIFIED_INPUTS`.
