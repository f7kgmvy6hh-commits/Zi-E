# Engineering Loops Applied to CAD v0.3

The CAD was not created as a single styling pass. The following loops are part of the release gate.

1. **Memory loop** — replayed the full approved project architecture, including details that are easy to lose between component selections: ESP32-S3 multimedia role, independent STM32 safety role, two-wheel base, Head Bay, service shell with no essential wiring, privacy/motion-kill controls, battery cassette, tool protocol, cable corridors and protected-shutdown sequences.
2. **Requirements loop** — every major subsystem is tied to a requirement/verification row.
3. **Source loop** — dimensions with high confidence come from manufacturer/distributor data; estimates remain labeled.
4. **Alternative loop** — conditional parts retain A/B/C fallbacks and switch conditions rather than pretending an untested choice is permanent.
5. **Packaging loop** — actual component envelopes drive body/head dimensions.
6. **Interface loop** — mechanical, electrical, data, power and service interfaces checked together.
7. **Swept-volume loop** — Head Active/Sleep/Shutdown, arm Active/Reach/Desk/Stow and wheel Deployed/Stowed are modeled from one source.
8. **Collision loop** — automated boolean checks cover the defined distal arm poses, head docking states and wheel/base interfaces.
9. **Tolerance/DFM loop** — moving/static/service clearances parameterized instead of hard-coded.
10. **Mass/COM loop** — an explicit mass model estimates the current ~2.4 kg prototype and ~91 mm active COM height; low-confidence values are tagged for measurement replacement.
11. **Stability loop** — baseline, one-arm-payload and two-arm-payload support-triangle load cases checked. The risky simultaneous dual-arm case becomes a software operating restriction.
12. **Power loop** — per-arm eFuse budgets and wheel/head rail limits are checked against the 3S2P cell ceiling instead of summing unrestricted servo stall currents.
13. **Common-cause loop** — optical cliff sensors are not treated as the only protection; bumper/anti-tip and fail-stop behavior remain independent.
14. **Cable loop** — hollow pan and dynamic head/arm cable keep-out volumes are part of CAD.
15. **Thermal loop** — rear/lower airflow paths, separate speaker chamber and battery/electronics separation are preserved; physical thermal testing still gates freeze.
16. **RF/EMI loop** — upper-rear ESP32 antenna keep-out and separation from high-current/magnetic areas preserved.
17. **Human-safety loop** — rounded covers, protected joints, spring-open gripper, no arbitrary AI actuator authority, hardware motion kill.
18. **Service loop** — rear service cover, battery door, modular boards, manual lift rescue and replaceable wheel/tool modules are preserved.
19. **Failure/recovery loop** — no head retract unless dock state is confirmed; wheel lock confirmation gates drive; extension/lift have independent end states; tool power is off until lock+ID.
20. **Red-team loop** — anything still requiring measured hardware is explicitly a freeze gate, not hidden by CAD precision.


## Open-Source Harvest Loop (added v0.3)
Problem → inspect mature open-source robots **and issue trackers** → extract failure pattern → adapt to ZI-E → simulate/bench test → record KEEP/REJECT. See `OPEN_SOURCE_HARVEST_AUDIT.md`.
