# Chronological Design / Conversation Decision Log

This is a project-relevant handoff log of the conversation that defined ZI-E Concept v0.1. It is intentionally structured rather than a verbatim UI transcript.

## Project workflow discussion

- User described a robotics project that will be fully prototyped before buying/assembling parts.
- Repository on GitHub is intended to be the persistent project base.
- Multiple AI systems will participate, including coding/review systems.
- ChatGPT is intended to coordinate, structure tasks, review outputs, and reduce credit waste.
- Project must later migrate cleanly to another ChatGPT account.
- Decision: GitHub/repository documentation should carry persistent project context instead of depending on one chat.

## Legacy material

- User supplied several older WALL-E-related theory/build/firmware files.
- User explicitly stated the theory material contains many errors.
- Decision: review work piece-by-piece and do not trust “Final/Corrected” labels.
- Later design discussion intentionally moved ahead of component review to prevent getting lost in premature electronics detail.

## AI brain concept

- User wants AI wirelessly connected from a laptop to the robot.
- Laptop is initially the AI/learning/database brain.
- As useful data/memory accumulates, a used phone may later be integrated.
- Direction: robot body handles local hardware and safety; laptop handles heavy intelligence.
- Memory/adaptation is preferred before considering continuous model retraining.

## Basic form factor

- User does not care about an exact tiny/large size, but wants a robot suitable for a desk.
- Direction: balanced desktop presence with enough room for battery, modular internals, and mechanisms.

## Face / head

- Mechanical eyes rejected as too much work/parts.
- Decision: digital eyes/face on a single landscape display.
- Face idea is somewhat analogous to a screen-based character concept, but ZI-E must remain original.
- Head should be custom, not a generic screen box.
- Head shape selected: custom soft-trapezoidal/futuristic shape, wider near top and narrower below.
- Screen orientation: landscape.
- Custom bezel selected.
- Camera should be hidden in head, top-center above display, and made more visually concealed.
- Updated concept image made the camera smaller/more hidden behind a dark/smoked area.
- Head proportion selected as balanced, roughly 55–60% of body width as a visual starting point.
- Head should be low/wide rather than tall.

## Head movement / states

- Camera placement makes head motion useful.
- Decision: Pan + Tilt.
- User proposed head lowering into body for relaxation/sleep.
- Central single neck selected over dual posts.
- Vertical internal slider/guide-rail mechanism selected over telescopic neck.
- Three physical states established:
  - Active: head raised, neck visible.
  - Sleep: head lowered enough to hide neck, face still visible.
  - Protected Shutdown: head lowers deeper and becomes almost fully protected/hidden.
- Protected Shutdown objective: maximum protection, smallest form, best appearance.
- Top body opening follows head shape.
- Upper surface should slope gently inward with a very subtle protective rim.

## Body design

- Hybrid body selected.
- Front should be convex to give ZI-E personality.
- Body should not become a rectangle/box.
- Rear should remain sculpted and attractive.
- Exterior should remain clean with hidden vents/sensors where possible.

## Heart RGB

- User wants a visible light source on the chest but not a visible decorative insert when off.
- Direction changed from obvious heart emblem to **hidden RGB core behind flush diffuser**.
- When RGB is off, exterior should look nearly seamless.
- Active/Sleep can use different pulse behavior.
- Shutdown turns it off.

## Wheels / locomotion

- User selected wheels rather than tracks.
- Need stability without self-balancing complexity.
- Early idea: two powered wheels with hidden caster/support.
- User explored steerable/360-degree wheels or ball movement.
- After reconsideration, decision returned to:
  - two fixed independent powered wheels;
  - differential drive;
  - hidden passive support;
  - zero-radius turning.
- Reason: sideways travel is not worth the mechanical penalty for ZI-E.
- Wheels should be visible, not fully hidden.
- Style selected: hidden-mechanical and futuristic.
- Semi-exposed concept selected: approximately half integrated in housing.
- Concept starting diameter: ~35–40% of body height.
- Protected Shutdown: wheels can tuck/fold inward **manually**, not with powered mechanism.
- Hidden support/feet can stabilize robot in compact state.

## Arms / hands

- Initial expressive-only arms were reconsidered.
- User wants arms to perform useful basic tasks and remain extensible.
- Current arm concept:
  - high shoulder mount;
  - shoulder joint;
  - elbow joint;
  - one-stage telescopic forearm;
  - modular wrist/tool interface;
  - compact adaptive two-finger gripper.
- Normal arm length should look natural; extension used only when needed.
- Concept extension target: around 135–150% of normal length, subject to engineering.
- Motion should look natural/easy rather than industrial/over-articulated.
- Wrist/tool interface:
  - manual quick-swap now;
  - designed to allow automatic tool changing later.
- Protected Shutdown sequence:
  - retract forearm;
  - close gripper;
  - safe wrist position;
  - fold elbow;
  - tuck shoulder;
  - arm nests into side recess.

## Internal access / modularity

- User wants the body to open so components can be accessed and changed comfortably without breaking wires.
- Decision: outer cosmetic shell + internal structural core.
- Major rear shell can be removed for service.
- Avoid wiring essential components to removable cosmetic covers.
- Multiple distributed modules are preferred over one huge tray.
- Each component/module should be placed in its best functional location.
- Internal zones proposed:
  - top: head mechanism;
  - center: control/communications;
  - bottom-center: battery/power;
  - sides: wheels/arms;
  - front peripheral: hidden heart/sensors.
- Service goal: one module should not unnecessarily block access to another.

## Battery

- User wants a large battery, long runtime, and future upgrade capability.
- Direction: large low/central battery zone.
- Quick-release door/cartridge inspired by camera batteries.
- Best location selected conceptually: lower rear, subtly angled and flush.
- It should be accessible without turning ZI-E upside down.
- Battery bay should allow a future larger pack.

## Audio

- Speaker should be hidden.
- User wants good quality and more surrounding sound rather than an ugly visible grille.
- Direction: internal acoustic chamber with carefully calculated vents/ducts.
- Vent geometry should be driven by real acoustic design, not random multiple openings.
- Candidate direction: output toward front/sides/bottom and make use of desk reflection.
- Speaker should be kept away from head microphone when practical.

## Concept image

Two concept-sheet iterations were produced.

The second iteration specifically corrected:
- chest RGB/core to be more hidden and seamless;
- camera to be more discreet.

Images remain visual inspiration and are not engineering drawings.

## Current stopping point

Concept design is sufficiently defined to begin **Design Freeze v0.1**.

Next work should convert visual proportions to real dimensions and internal movement/service envelopes before choosing parts one-by-one.
