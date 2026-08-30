# Host / ESP32 Interface V1

Status: semantic contract candidate; transport binding disabled until authenticated,
commissioned device identity and a verified board profile exist.

An authenticated persistent WebSocket is the target candidate. Exact encoding is not
frozen. Logical planes are CONTROL (bounded structured messages), AUDIO (binary Opus),
VISION (capture request plus bounded upload/reference/stream), and EVENTS (structured
health/state). Large image/audio base64 does not belong in control JSON.

Every command carries `session_id`, `generation_id`, `command_id`, semantic type,
bounded payload, and expiry/lease semantics where applicable. Reconnect establishes a
fresh session/generation; duplicate or retired identities do not renew authority.
Connection or authentication alone never grants motion authority.

Host-to-ESP32 semantics may map existing Robot/Presentation APIs to face intent, RGB
effect, audio session/playback, camera capture request, robot intent, and STOP request.
ESP32-to-Host events may report device/network/audio/face/camera/controller-link state,
wake/button observations, intent acceptance, execution lifecycle, and faults. Names
remain mapping candidates, not a competing enum set.

Lifecycle is `RECEIVED -> ACCEPTED -> EXECUTING -> COMPLETED`, or typed
`REJECTED/CANCELLED/FAULTED`. Only authoritative copied STM32 feedback may report
physical execution. ESP32 may reject locally but cannot report acceptance as completion.

The interface has no raw socket endpoint in the HUD and no GPIO/PWM/register/raw-CAN
or actuator units. Camera/audio/face/MCP/providers cannot create robot authority.
Host loss retires command authority and leaves ESP32 presence behavior non-motion.

See `APP_HOSTRUNTIME_ADAPTER_CONTRACT.md`, `SEMANTIC_ROBOT_API_FOUNDATION.md`,
`DEVICE_IDENTITY_FOUNDATION.md`, and `HARDWARE_PROFILE_RESOLUTION_FOUNDATION.md`.
