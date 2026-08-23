# Current State — 2026-08-23

## Phase
Prototype architecture and CAD package prepared; next phase is physical validation/bench rigs followed by electronics schematics/PCB and firmware integration.

## What is substantially defined
- Overall structural architecture and service philosophy.
- Digital face / camera / audio architecture.
- ESP32-S3 multimedia and STM32 safety/motion split.
- Head Pan/Tilt/Lift mechanisms and operating states.
- Two-wheel differential base, folding wheel pods, passive caster, shutdown feet.
- Dual functional arm architecture with candidate actuators.
- Adaptive gripper and magnetic quick-swap tools.
- Battery placement/configuration, V1 charging approach and protected power-domain philosophy.
- Cliff/collision sensing layers and safety state machine direction.
- Parametric CAD, STEP/STL exports, CAD FMEA/requirements/validation dataset.

## Current CAD snapshot
- 34 automated checks; 33 pass; 1 restricted case.
- Estimated mass ~2.42 kg pending measured masses.
- Active modeled height ~282 mm; Shutdown modeled height ~182 mm.
- Two-arm simultaneous 250 g forward payload is not allowed during normal driving under the current support geometry.

## Main unresolved gates
See `OPEN_ITEMS_AND_FREEZE_GATES.md`: display exact interface, physical cliff testing, real arm/head mass and torque/thermal tests, wheel traction/tire choice, magnet force/temperature, battery protection sizing, audio and cable-life tests.

## Rule for future work
Every new answer/change must use the Memory/Inventory Loop plus the full engineering loop and update repository source-of-truth docs when approved.
