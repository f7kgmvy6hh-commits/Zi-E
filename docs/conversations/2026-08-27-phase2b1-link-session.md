# 2026-08-27 — Phase 2B1 Link Session

## User direction

- Continue from commit `3afa18a`; preserve completed Phase 1 and Phase 2A work.
- Advance one dependency-safe foundation toward an extensible, safety-bounded robot
  platform; stop at a coherent READY_TO_COMMIT boundary.

## Work completed

- Verified the clean `refactor/modular-hardware-architecture` baseline and inspected
  the committed Phase 2A implementation.
- Defined Phase 2B1 separately from the still-gated wire/transport work.
- Added a transport-independent controller session/liveness guard with protocol
  compatibility, configured peer identity, peer boot-session binding, ordered
  heartbeats, deterministic expiry, and explicit renegotiation.
- Added host assertions for incompatible/wrong peers, duplicate/stale/session-mismatched
  heartbeats, wraparound, expiry, peer restart, and authority removal.
- Remediated independent safety review findings: Hello now creates only a candidate;
  fresh valid heartbeat liveness is required for authority, including after
  renegotiation. Stray/incompatible Hellos are rejected without tearing down a healthy
  session, while an expected-peer boot change still faults as a restart.
- Created the risk register and future-suggestions parking lot required by the project
  process; recorded the protocol risks and a deferred trace-replay tool idea.

## Validation status

- Full diff review and `git diff --check` completed.
- External host validation completed under MSYS2 UCRT64 with GNU g++ 16.1.0,
  CMake/CTest 4.3.3, and Ninja 1.13.2.
- `cmake --build "/tmp/zie-build-a2c6ef9"` passed, followed by
  `ctest --test-dir "/tmp/zie-build-a2c6ef9" --output-on-failure`: 1/1 tests passed,
  zero failed.
- CAN/TWAI framing, integrity, timing, physical safe stop, bench, and HIL validation
  remain unresolved and unverified. Long-lived authenticated replay protection remains
  explicitly open for Phase 2B2.

## Post-commit host-test correction

- Phase 2B1 was committed as `a2c6ef9`.
- The first available C++17/CTest run exposed an incorrect Phase 2A test expectation,
  not a lifecycle implementation defect: from `0xFFFFFFF5` to `5`, unsigned elapsed
  time is 16 ms, while `9` is the exact 20 ms lease boundary.
- The test now asserts no expiry at `5`, expiry at `9`, and acceptance of the newer
  wrapped sequence afterward. `MotionCommandLifecycle` was not changed.
- The required external rebuild and CTest rerun passed with 1/1 tests and zero
  failures. The correction is ready to commit.
