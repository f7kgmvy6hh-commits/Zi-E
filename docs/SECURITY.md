# Security

V1 binds only to loopback and requires a bearer token of at least 24 characters for
every API and WebSocket connection. Secrets stay in git-ignored `.env`; public
settings, route events, and audit records never include credentials. Hermes owns
provider OAuth material.

Subprocesses use argument arrays. Robot control exposes semantic transitions only.
Do not port-forward the server, disable authentication, commit `.env`, or expose raw
actuator interfaces to the chat layer.
