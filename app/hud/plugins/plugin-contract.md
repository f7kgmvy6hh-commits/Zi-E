# Zi-E HUD plugin contract

Each HUD plugin is a declarative module exposed through `window.ZIE_PLUGINS`.

Required fields:

- `id`: stable panel identifier.
- `title`: operator-facing display name.
- `text`: truthful initial status; never invent hardware telemetry.
- `permission`: `read` for informational surfaces; privileged actions remain backend-only.
- `events`: event types the plugin may subscribe to.

The browser receives only the local Zi-E bearer token and public event/state data.
Plugins must not access Hermes credentials, provider keys, arbitrary environment values,
or unrestricted actuator APIs. Robot actions must use the authenticated Zi-E backend,
which enforces deterministic safety checks and emergency-stop behavior.
