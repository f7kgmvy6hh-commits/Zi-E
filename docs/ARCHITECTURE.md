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
