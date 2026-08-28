# 2026-08-28 — Stale context repair and hardware-profile resolution foundation

The Product Owner requested one coherent follow-on stage from `ce7ab88`: first repair
stale Plugin SDK context metadata, then complete an in-memory data-driven hardware
profile and resolution boundary. Dynamic loading, discovery, parsers, drivers,
transport, persistence, commissioning, and physical hardware work were excluded.

Implemented direction:

- `ExtensionContext` is explicitly an immutable issuance snapshot. Retained old
  metadata is historical only and every retained service fails stale after revocation.
  Current host queries expose zero-grant non-active snapshots after suspension,
  failure, quarantine, or recovery; removal returns null; explicit activation issues
  a fresh context and epoch.
- Hardware profiles carry trusted stable identity, exact schema `1.0.0`, revision,
  platform, owner/source, semantic hardware entries, requirements, cardinality,
  compatible categories, ownership, and safe constraints. Unknown or raw domains fail
  closed.
- Resolution consumes registry-authoritative copied candidate facts and matches exact
  profile/category/lifecycle/capability/ownership identity deterministically. Required
  absence and ambiguity block activation; optional absence is retained diagnostically.
- Profile activation is atomic. Revocation invalidates the selected resolution and
  never silently substitutes another device; explicit re-resolution and reactivation
  are required.
- Plugin SDK contexts receive the authoritative device/profile binding already owned
  by the registry. Profiles and plugin configuration cannot grant trust, identity,
  controller, capability, protected safety, or raw hardware authority.

The open-source harvest reviewed ros2_control lifecycle documentation and real reports
covering multi-device misrouting, inactive hardware remaining claimed, and
nondeterministic staged startup. Zi-E adapts exact logical binding, active-only
resolution, deterministic ordering, and explicit staged activation; no external code
was copied.

During validation, CTest initially reported Windows exit code `0xc0000409`. A fresh
rebuild showed this was an assertion abort, not a memory-corruption fault: the missing-
battery fixture had accidentally used registry-managed ownership even though battery
is intentionally restricted to the protected safety core. The fixture was corrected
to use protected ownership; the safety validation and assertions remain intact. The
temporary diagnostic logging was removed after the cause was confirmed.

Final validation used fresh MSYS2 UCRT64 GNU g++ 16.1.0 C++17 configure/build
trees. Normal and warning-clean (`-Wall -Wextra -Wpedantic -Wswitch-enum`) builds
passed, both CTest runs passed 1/1, and `git diff --check` passed.

Phase 2B2 remains `WAITING_FOR_VERIFIED_INPUTS`. This session stops at
`READY_TO_COMMIT`; no commit or push was requested.
