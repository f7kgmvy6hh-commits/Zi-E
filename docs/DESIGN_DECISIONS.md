# ZI-E Design Decisions — Concept v0.1

This document captures the design decisions made so far. Dimensions are not final unless explicitly stated.

## 1. Overall category and personality

- ZI-E is a **desktop companion robot**.
- It should not be extremely small or large.
- Exact size is secondary to good proportions, internal service space, battery capacity, and a clean desktop presence.
- Visual direction: **cute + futuristic + premium**, but believable and buildable.
- It should have its own identity rather than being a WALL-E or BMO copy.

## 2. Overall body shape

**Approved direction: Hybrid body.**

- Wider toward the bottom.
- Slightly narrower through the middle.
- Soft transitions toward the upper head opening.
- Rounded, non-boxy edges.
- Front chest is **gently convex** to give the robot personality.
- Rear remains sculpted and attractive; it must not look like a flat electronics access panel.

## 3. Head

### Shape
- Custom soft-trapezoid / futuristic head.
- Wider near the top and slightly narrower toward the bottom.
- Rounded edges.
- Balanced head size, not oversized-cute or tiny.
- Target proportion: roughly **55–60% of body width** as a concept guide.
- Head should be relatively low/wide to suit the landscape face and retraction.

### Face
- **One landscape digital screen.**
- Face is digital; no mechanical eyes or eyelids.
- Custom bezel around the display.
- Digital expressions can change without extra mechanics.

### Camera
- Camera belongs in the **top-center region above the screen**.
- It should be highly concealed.
- Preferred appearance: hidden behind a dark/smoked sensor window or very discreet slit.
- Camera remains usable in Active and Sleep modes.
- It becomes protected/hidden in full shutdown.

## 4. Head and neck motion

### Neck
- One central, short, relatively broad neck.
- Internal wiring should route through the neck where practical.

### Head motion
- Pan.
- Tilt.
- Vertical retract.

### Retraction mechanism
**Approved concept:** a vertical slider on internal guide rails.

Avoid a complicated telescopic neck for Version 1.

## 5. Operating poses

### Active Mode
- Head fully raised.
- Neck visible.
- Pan/Tilt available.
- Full visual interaction.

### Sleep Mode
- Head lowers enough for the neck to disappear.
- Display remains visible.
- Camera remains functional.
- Screen may show sleepy eyes, time, battery, or other low-energy information.
- Arms rest in a relaxed state.
- Hidden RGB heart/core uses a slower, dimmer pulse.

### Protected Shutdown
Goal: **maximum protection + minimum physical volume + attractive compact form**.

- Head retracts significantly deeper than Sleep.
- Screen and camera become physically protected.
- Head should be almost completely hidden inside the body.
- Top opening follows the head's shape.
- Body has a slight inward slope and a subtle protective rim around the head opening.
- Arms retract/fold into side recesses.
- Wheels can be manually tucked/folded inward.
- Robot should resemble a compact protected pod rather than a robot merely switched off.
- RGB heart/core turns off.

## 6. Top body opening

- Opening follows the custom head shape rather than being a generic rectangle.
- Upper surface slopes slightly inward toward the opening.
- Very subtle raised/protective rim.
- Aim for a nearly closed/flush-looking protected shutdown profile.

## 7. Chest light / “heart”

- RGB status/emotional light on the front chest.
- Location should be easy to see.
- **It must be hidden when off.**
- No obvious external heart badge when inactive.
- Preferred implementation concept: internal RGB light behind a flush, frosted/seamless diffuser integrated into the convex chest.
- Active: subtle expressive pulse.
- Sleep: slower/dimmer pulse.
- Shutdown: off.
- Future status colors may indicate charge/errors without making the chest look like a dashboard.

## 8. Drive system

### Approved base architecture
- Two **fixed powered wheels**, independently driven.
- Differential drive.
- Hidden passive support point/caster underneath.
- Zero-radius turning using opposite wheel directions.
- No swerve/360-degree steering in Version 1.
- No motorized ball drive.
- No Mecanum/omni base in Version 1.

Reasoning: sideways travel is not worth the extra mechanical complexity, size, wiring, backlash, and failure points for this companion robot.

The independently moving head can maintain gaze while the body reorients.

### Wheel appearance
- Semi-exposed.
- Roughly half visually integrated into the housing and half visible as a concept direction.
- Medium-to-large visual proportion, approximately **35–40% of body height** as a starting proportion only.
- Style: **hidden mechanical + futuristic**.
- Motors/axles/wiring remain concealed.
- Wheel pods should look designed into the body, not attached afterward.

### Protected shutdown wheels
- Manual inward tuck/fold is acceptable and desired.
- No powered wheel-fold mechanism for Version 1.
- Robot may rest on hidden rubber feet when wheels are tucked.

## 9. Arms

The arms are **functional and expressive**, not decorative only.

### Placement
- Shoulder mounts are high on the sides.

### Joints / functions
- Shoulder joint.
- Elbow joint.
- Telescopic forearm, one extension stage.
- Compact wrist/tool interface.
- Compact adaptive two-finger gripper as the normal/default hand.

### Arm reach
- Normal arm length should look proportional in daily use.
- Forearm extends only when extra reach is needed.
- Concept target: extension to roughly 135–150% of normal length, subject to engineering validation.

### Hand/gripper
- Two-finger adaptive gripper.
- Rounded and compact.
- Designed for light tasks such as:
  - holding small/light objects;
  - pressing buttons;
  - passing an object;
  - holding a stylus through an adapter;
  - pushing/pulling light items.

This is not intended as a heavy industrial manipulator.

### Wrist / tool ecosystem
- Manual quick-swap tool system for Version 1.
- Interface should be designed **auto-ready** for a possible future docking/tool-changing system.
- Future tool examples: stylus, sensor, magnetic pickup, suction tool, small light, custom tools.
- Mechanical lock, power/data contacts, and alignment concept will be engineered later.

### Arm shutdown sequence
1. Retract telescopic forearm.
2. Close gripper.
3. Return wrist to safe position.
4. Fold elbow.
5. Tuck shoulder.
6. Entire arm nests into a shaped side recess.

The recess should look like a natural body design line rather than an ugly cavity.

## 10. Internal service architecture

### Exterior vs structure
ZI-E uses:
- cosmetic outer shell;
- structural internal core/frame;
- modular internal subsystems.

The cosmetic shell should not be the sole load-bearing structure.

### Rear access
- Sculpted rear appearance.
- Rear shell should be removable for major service.
- No essential wiring should live on a removable cosmetic cover where it must be unplugged every time.
- Seams should follow design lines.

### Internal modularity
Use multiple modules placed in their optimal functional locations rather than one giant electronics tray.

Candidate zones:
- top: head lift / head motion;
- center: controller and communication;
- bottom-center: battery/power;
- sides: wheel modules and arm mechanisms;
- front peripheral: hidden RGB/core and forward sensors.

Modules should be removable independently where practical.

## 11. Battery concept

- Large battery capacity is a priority.
- Long runtime matters more than shrinking the body by a few centimeters.
- Battery bay should support later capacity upgrades.
- Battery lives low and central for a low center of gravity.
- Preferred access: **camera-style quick-release battery cartridge/door**.
- Best concept location: lower rear, on a subtly angled flush panel.
- User should not need to turn the robot upside down to access it.
- Door should not visually damage the rear design.

Battery chemistry/cell configuration is NOT yet selected.

## 12. Audio

- No ugly front speaker grille.
- Speaker should be hidden inside the body.
- Use an internal acoustic chamber and carefully designed vents/ducts.
- Consider symmetric output paths and desk-surface reflection.
- Do not assume “more holes = better sound”; final acoustic dimensions depend on the real speaker/enclosure.
- Keep speaker physically separated from the head microphone as much as practical.
- Software echo cancellation may be used later.

## 13. Exterior cleanliness

From the front, visible elements should be minimal:
- digital face;
- subtle camera location;
- hidden-when-off RGB heart/core;
- intentional body surfaces.

Speaker openings and sensors should be hidden in the best functional locations using vents, dark windows, seams, or lower/rear openings.

## 14. Serviceability principle

ZI-E should be upgradeable without redesigning the whole robot.

Examples:
- controller replacement;
- battery upgrade;
- wheel module replacement;
- arm/tool upgrade;
- speaker change;
- added sensor;
- future phone integration.

Prefer standardized internal mounting patterns and connectors.

## 15. Concept-image status

The repository contains concept sheets under `assets/concepts/`.

Important correction applied to later concept:
- camera made much less visible;
- RGB chest light made concealed/seamless when off.

Images are **visual direction references**, not dimensionally accurate CAD.


## 2026-08-23 — Hidden Belly Light Matrix and pre-purchase failure-harvest
**Decision:** Add a hidden RGB belly matrix behind a flush secret-until-lit optical insert. Prototype with IS31FL3741 13×9, but reserve larger custom matrix keepout in CAD. It is non-safety multimedia and must fail dark.

**Decision:** Add a permanent Open-Source Harvest Loop. Reachy Mini, LeRobot SO-101, OpenArm, Stretch, MoveIt/Nav2 and MuJoCo failure patterns are used to create preventive tests and architecture changes before bulk part purchase. Immediate changes include safe commissioning, explicit command execution state, single motion arbiter, bus/cable fault containment, dynamic payload/stability model, dynamic collision zones, diagnostic tooling and explicit simulation inertials.

## 2026-08-23 — Project v0.3 dedicated head laser rangefinder
- Add one dedicated forward single-point Class-1 ToF laser rangefinder in the head sensor brow.
- Primary candidate: VL53L1X, because up-to-4 m / 50 Hz is sufficient for a desktop companion and the final bare package is extremely small.
- Camera remains centered; rangefinder is offset and software must calibrate the camera/rangefinder extrinsic relationship.
- Rangefinder is for measurement/vision assistance and does **not** replace local STM32 cliff/proximity/bumper safety.
- The optical brow/window must be validated for ~940 nm transmission and crosstalk; arbitrary dark plastic is not accepted without test.
- TF-Luna becomes the intentional backup if >4 m range or >50 Hz is required; TFmini-S only if a real longer-range requirement appears.

## 2026-08-27 -- Modular hardware firmware boundary

- Preserve ESP32-S3 ownership of multimedia/presence hardware and STM32 ownership of
  safety/motion-critical hardware.
- AI/behavior uses a safe high-level command facade and never directly controls GPIO,
  PWM, registers, vendor APIs, motors, or individual actuators.
- Hardware is supplied through stable capability interfaces and explicit profile
  bindings. Optional hardware is represented as an absent capability, not hidden by a
  guessed or silent production fallback.
- Production motion remains gated by verified low-power commissioning and actual-state
  confirmation. The initial code is an architecture scaffold, not a hardware freeze.

## 2026-08-27 — Phase 2A motion lifecycle foundation

- Establish the command lifecycle as a transport-independent core before selecting a
  CAN/TWAI wire encoding.
- Identify motion commands by source, boot/session, and sequence; never use receipt
  order alone as proof of a new command.
- Permit one active motion command owner per subsystem. Duplicates are idempotent and
  do not renew leases; implicit replacement of an in-flight command is rejected.
- Check the monotonic lease on every state-changing call. Expiry is a local fault that
  requires a physical safe stop and later state confirmation by STM32.
- Defer cross-session authority negotiation, replay history, wire framing, heartbeat,
  and controller-reboot handling to Phase 2B.

## 2026-08-27 — Phase 2B1 controller session and liveness

- Negotiate controller compatibility by independently versioned protocol major/minor;
  a major mismatch fails closed and the negotiated minor is the lower supported value.
- Bind an active link to the configured peer controller and its nonzero boot session.
  A changed boot session removes motion authority and requires explicit local
  renegotiation; receipt of a new hello alone cannot silently replace the peer.
- A compatible expected-peer Hello establishes only a candidate session. Motion
  authority remains unavailable until the first valid bound heartbeat proves liveness;
  every renegotiated session requires a fresh heartbeat.
- Reject malformed, unexpected-controller, retired-session, and incompatible-version
  Hellos without mutating a healthy established session. Only a different boot session
  from the configured expected peer triggers a restart fault. This reduces trivial
  unauthenticated Hello denial of service without claiming authentication.
- Duplicate, stale, wrong-session, and invalid heartbeats do not renew liveness.
- Supply the monotonic heartbeat timeout from verified configuration. Do not invent a
  production timeout in reusable protocol code.
- Defer wire encoding, CAN/TWAI framing, integrity selection, and physical safe-stop
  binding until bus analysis and bench evidence exist. Long-lived authenticated replay
  protection remains explicitly open for Phase 2B2.

## 2026-08-27 — Extension manifest and capability trust foundation

- Treat `HOST_PLUGIN`, `ASSET_PACK`, `EMBEDDED_MODULE`, and
  `PROTECTED_SAFETY_MODULE` as separate execution models with class-specific validation.
- Assign trust from the registry/install source; never accept a manifest's claim that it
  is built-in, signed, or otherwise trusted.
- Treat requested permissions as declarations for a future deny-by-default enforcement
  layer, not grants. No raw actuator/safety-bypass permission exists; host motion access
  means Safe Robot API requests only.
- Keep declared, validated, and active capabilities separate. A manifest claim cannot
  populate either validated or active state, and inactive lifecycle states expose none.
- Version extension package, Plugin API, and manifest schema independently.
- Keep the current implementation as an in-memory validation contract. Parsing,
  loading, sandboxing, signatures, registry state transitions, and configuration
  activation require later reviewed foundations.
