# ESP32 / STM32 Semantic Interface V1

Status: responsibility and semantic boundary candidate. Phase2A and Phase2B1 remain
the executable host-tested foundation. Phase2B2 is `WAITING_FOR_VERIFIED_INPUTS`.

ESP32 submits only bounded semantic intents such as base move/stop, head look/home,
arm pose/stow, grip, tool action, and motion stop. STM32 validates identity, state,
limits, local safety, freshness, and feasibility and may always reject or stop.

Each command preserves `session_id`, `generation_id`, `command_id`, semantic payload,
and a verified bounded lifetime. A hello/candidate or transport connection does not
grant authority; a fresh authorized session is required. Retired sessions,
generations, duplicates, stale commands, partial messages, malformed data, peer reset,
or timeout fail closed. STOP is idempotent and prioritized. Execution lifecycle keeps
`ACCEPTED` distinct from `EXECUTING` and `COMPLETED`.

The boundary forbids PWM, GPIO, raw servo/timer/register values, arbitrary setpoints,
raw CAN transmit, and safety disable. Trajectory generation, actuator representation,
encoders, PWM/current control, hard limits, and local safe stop stay below the STM32
boundary.

No CAN bitrate, IDs, pins, transceiver, termination, packing, fragmentation, queue,
heartbeat, or safe-stop timing is selected here. Those require verified ESP32/STM32
hardware, controller clocks, electrical topology, message inventory, response-time
analysis, and bench fault evidence under `PHASE2B2_INPUT_GATE.md`.
