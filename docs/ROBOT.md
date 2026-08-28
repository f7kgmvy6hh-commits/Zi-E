# Robot Operation

Real mode is the default and reports no invented telemetry. Commands are semantic
high-level requests through the local safety state machine. Emergency stop is always
reachable; leaving it requires an explicit operator reset to SAFE. Deadman expiry
also returns the controller to SAFE.

Do not enable autonomous motion until low-power commissioning verifies actuator
identity, voltage class, bus and baud, direction, zero and range, end-state sensors,
and plausible feedback. Command receipt is not execution; actual feedback must
confirm completion. The example robot YAML intentionally leaves all gates false.
