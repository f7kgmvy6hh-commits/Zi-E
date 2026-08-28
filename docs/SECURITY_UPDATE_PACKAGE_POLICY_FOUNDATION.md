# Security, Update, and Package Policy Foundation

Status: implementation complete; ready for commit review (2026-08-28).

This stage adds bounded in-memory authority for package identity, external
verification, compatibility, activation, atomic update, rollback, quarantine,
secret boundaries, and watchdog decisions. Package ID, version, and content digest
abstraction remain distinct; path/URL identity and self-declared trust/controller/
profile authority fail closed. Protected safety is built-in-only and asset packs are
data-only.

ExtensionHost can reject activation lacking package-policy approval. Updates and
rollbacks suspend old authority, replace the registry version at the current
generation, and use ordinary ExtensionHost activation for a fresh SDK epoch.
HostRuntime can require live package readiness during startup, refresh, and recovery.
The watchdog exposes typed semantic actions only, while credentials remain opaque,
owner-scoped handles and protected configuration namespaces are rejected.

No production cryptography, keys, networking, installer/archive handling, dynamic
loading, persistence, UI, scheduler, real credentials, driver, CAN/framing,
autonomous update, or commissioning behavior is implemented.
