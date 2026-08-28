# System Architecture — Direction v0.1

## Principle

ZI-E's body should remain a safe real-time robot controller, while heavy AI runs outside the body initially.

## Initial system

```text
                       ┌──────────────────────────────┐
                       │          LAPTOP              │
                       │                              │
 Camera/Audio/Sensors ─► Vision / Speech / LLM       │
                       │ Behavior / Memory / Database │
                       │                              │
 Safe intents/commands ◄                              │
                       └──────────────┬───────────────┘
                                      │ Wi-Fi
                                      │
                       ┌──────────────▼───────────────┐
                       │          ZI-E BODY           │
                       │                              │
                       │ Robot controller             │
                       │ Motor/servo control          │
                       │ Local safety                 │
                       │ Telemetry                    │
                       │ Camera/mic transport         │
                       │ Sensor transport             │
                       └──────────────────────────────┘
```

## Local robot responsibilities

Expected:
- Wi-Fi communications;
- actuator control;
- sensor acquisition;
- camera/microphone data transport;
- telemetry;
- emergency stop;
- collision / motion safety;
- state machine;
- safe command validation;
- local fallback behavior where useful.

Not intended as the primary responsibility:
- main LLM;
- long-term personal memory;
- main vector/database system;
- unrestricted AI decisions;
- cloud secrets embedded in distributable firmware.

## Laptop brain responsibilities

Candidate responsibilities:
- speech recognition;
- text-to-speech orchestration;
- vision inference;
- LLM reasoning;
- long-term memory;
- user preferences;
- episodic memory;
- behavior planning;
- database;
- learning/adaptation logic;
- diagnostics and development tooling.

## Memory direction

Prefer adaptation through memory/retrieval before considering continual model retraining.

Possible memory layers:
1. Immediate/context memory.
2. Episodic interaction memory.
3. Long-term people/preferences/places/configuration/behavior memory.

## Future used-phone integration

A used phone may eventually become:
- portable edge brain;
- local database host;
- connectivity bridge;
- camera/IMU/GPS resource;
- speech/vision processing device;
- cloud AI client;
- replacement or partial replacement for the laptop.

Design should preserve room for this architecture without requiring the phone to be physically installed in Version 1 unless later decided.

## Hardware control safety boundary

External AI must request **high-level intents**.

Good examples:
- `MOVE_FORWARD`
- `TURN_LEFT`
- `STOP`
- `WAVE`
- `LOOK_AT_TARGET`
- `SLEEP`

Avoid exposing uncontrolled raw hardware operations directly to AI, such as arbitrary raw PWM/servo values without local safety checks.


## Hidden Belly Light Matrix
A fixed-torso secret-until-lit RGB matrix is controlled by ESP32-S3 as non-safety multimedia. It has independent power gating/current limiting and must not share a critical failure dependency with cliff/safety sensing.

## Commissioning / arbitration
Normal autonomous motion is locked out until actuator identity/direction/range checks pass. High-level control is single-owner/lease based and motion commands distinguish receipt from physical execution.


## Head measurement sensor
`VL53L1X -> I2C -> ESP32-S3` provides single-point forward distance aligned/calibrated with the head camera. It is non-safety-critical. Lower cliff/proximity sensors remain local to STM32.

## V1 command-center integration

The loopback server owns authentication, audit, state, events, voice orchestration,
and the HUD. Language-model work runs through installed Hermes 0.20.6 using the
stable named session and supported contract `hermes chat --continue NAME
--create-if-missing -q MESSAGE -Q --source tool`.

ZI-E deterministically routes simple prompts to `minimax-oauth/MiniMax-M3` and
complex prompts to the configured main model, defaulting to
`openai-codex/gpt-5.6-sol`. Explicit `/model` requests bypass automatic selection.
Route events expose provider/model names but never credentials.

Hermes configuration may contain `smart_model_routing`, but in this installed
version it is only a setup placeholder and is not consumed by the ZI-E bridge. No
unsupported Hermes smart-routing behavior is assumed.
