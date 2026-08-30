# ZI-E V1 Setup

Run `scripts/setup_zi-e.ps1` in PowerShell. It creates `.venv`, installs pinned
dependencies, and creates a git-ignored `.env` with a random bearer token. Configure
`ZIE_HERMES_COMMAND=hermes`; Hermes 0.20.6 must already be authenticated separately.

Optionally enable Camofox with `ZIE_CAMOFOX_ENABLED=true`. Start with
`scripts/start_zi-e.ps1`; it starts enabled dependencies and the server, waits for
authenticated health, then opens the HUD. Validate with `health_zi-e.ps1` and stop
with `stop_zi-e.ps1`. The short legacy script names remain wrappers.

For the complete host/ESP32/STM32 preparation flow, start with
`scripts/toolchain-doctor.ps1`, then use the fixed host actions in
`scripts/firmware-workflow.ps1`. Target actions require a reviewed board profile and
explicit device identity; see `DEVELOPMENT_PIPELINE_READINESS.md`. Vendor SDKs remain
outside this repository.
