# ZI-E Project Context

## Project intent

ZI-E is a **desktop AI companion robot** intended to feel alive, useful, upgradeable, serviceable, and visually distinctive without becoming unnecessarily mechanically complex.

The project will be prototyped comprehensively before the full hardware purchase. GitHub should be the persistent project memory so work can migrate between AI accounts and tools without losing decisions.

## Core philosophy

- Design first, but remain component-aware.
- Avoid committing to hardware before physical and functional requirements are clear.
- Prefer modularity and serviceability.
- Keep the exterior clean while making the interior easy to access and upgrade.
- Minimize unnecessary mechanical complexity.
- Treat AI as a high-level intelligence layer, not as a raw hardware driver.
- Local firmware must enforce safety limits.
- Preserve future upgrade paths without over-engineering Version 1.

## AI / compute direction

The initial intelligent system is not intended to run entirely inside the robot.

### Initial architecture

```text
ZI-E Robot Body
  sensors / camera / microphone / actuators
                 │
              Wi-Fi
                 │
                 ▼
Laptop AI Brain
  vision / speech / LLM / memory / database / behavior
```

The laptop is initially responsible for the heavy AI workload and long-term memory.

### Future architecture

Once the system has a useful database and stable behavior, a **used phone** may become an edge brain / portable compute layer and reduce dependence on the laptop.

The phone may later provide compute, storage, connectivity, camera/IMU/GPS resources, or serve as a bridge to cloud AI.

## Project management direction

The user is the product owner.

ChatGPT acts as the coordinating architect/reviewer: organize work, break it into tasks, prepare prompts for other AI systems, review outputs, avoid duplicated work, manage context, and protect the source of truth.

Other AI systems may include Codex, Claude, and Gemini. They should be assigned narrowly scoped work with clear ownership rather than all editing the same subsystem simultaneously.

## Migration requirement

The project must remain portable to another ChatGPT account later.

Therefore:
- important decisions belong in this repository;
- AI chats are not the sole project memory;
- current state must be maintained in `docs/CURRENT_STATE.md`;
- decisions must be recorded in `docs/DESIGN_DECISIONS.md`;
- legacy material must remain clearly separated from approved design.


Current process also includes an Open-Source Harvest Loop: inspect mature robotics projects and their real issue trackers before freezing important hardware/software decisions.


## v0.3 addition — head laser distance measurement
Dedicated forward VL53L1X Class-1 ToF rangefinder added to the head sensor brow; it is a high-level measurement/vision aid and does not replace STM32 cliff/bumper safety. See `docs/LASER_RANGEFINDER_SPEC.md`.
