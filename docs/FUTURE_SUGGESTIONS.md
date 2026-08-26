# ZI-E Future Suggestions

This is an unapproved parking lot. Entries are not architecture decisions and must not
be implemented without Product Owner review and the applicable evidence/freeze gates.

| ID | Title | Category | Problem/opportunity | Proposed idea | Why it may help Zi-E | Dependencies | Risks | Security/safety impact | Hardware impact | Software impact | Complexity | When to reconsider | Evidence/source | Status |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| FS-DX-001 | Protocol trace replay tool | Developer Experience | Controller reboot, ordering, and timeout failures are difficult to reproduce | Add a host tool that records sanitized semantic frames and replays deterministic fault cases into the virtual Zi-E link | Makes regressions and field diagnostics reproducible without hardware | Frozen bounded wire schema and secret-redaction rules | Logs may leak identifiers or become mistaken for hardware proof | Must redact secrets; replay must never connect to live actuator authority | None | Recorder, validator, virtual-link replay runner | MEDIUM | After Phase 2B2 wire schema is stable | OpenCyphal ordering concepts; micro-ROS reconnect failure reports | DEFERRED |
