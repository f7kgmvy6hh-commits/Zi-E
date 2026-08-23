# Current State — 2026-08-23

## Phase
Prototype architecture and Project/CAD v0.3 package prepared; next phase is risk-retirement bench rigs / safe commissioning prototypes before bulk component purchase, followed by electronics schematics/PCB and firmware integration.

## What is substantially defined
- Overall structural architecture and service philosophy.
- Digital face / camera / dedicated head laser-rangefinder / audio architecture.
- Hidden secret-until-lit Belly Light Matrix (BLM) for RGB ambient light + low-resolution information such as clock/battery/status.
- ESP32-S3 multimedia and STM32 safety/motion split.
- Head Pan/Tilt/Lift mechanisms and operating states.
- Two-wheel differential base, folding wheel pods, passive caster, shutdown feet.
- Dual functional arm architecture with candidate actuators.
- Adaptive gripper and magnetic quick-swap tools.
- Battery placement/configuration, V1 charging approach and protected power-domain philosophy.
- Cliff/collision sensing layers and safety state machine direction.
- Parametric CAD, STEP/STL exports, CAD FMEA/requirements/validation dataset.

## Current CAD snapshot
- Project/CAD v0.3: 39 automated checks; 38 pass; 1 restricted case.
- Dedicated VL53L1X head laser-rangefinder envelope added beside the centered camera.
- Estimated mass ~2.448 kg pending measured masses.
- Active modeled height ~282 mm; Shutdown modeled height ~182 mm.
- Two-arm simultaneous 250 g forward payload is not allowed during normal driving under the current support geometry.

## Main unresolved gates
See `OPEN_ITEMS_AND_FREEZE_GATES.md` and `OPEN_SOURCE_HARVEST_AUDIT.md`: display exact interface, laser optical-window/crosstalk + camera alignment, safe actuator commissioning, bus/cable fault containment, physical cliff testing, real arm/head mass and torque/thermal tests, wheel traction/tire choice, magnet force/temperature, battery protection sizing, audio/cable-life tests, and belly optical/thermal validation.

## Rule for future work
Every new answer/change must use the Memory/Inventory Loop + full engineering loop + Open-Source Harvest Loop, and update repository source-of-truth docs when approved.
