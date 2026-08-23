# ZI-E Roadmap

## Phase A — Concept Design
Status: substantially complete.

- visual personality;
- operating poses;
- modularity direction;
- basic system architecture;
- service philosophy.

## Phase B — Design Freeze v0.1
Next.

- overall mechanical envelope;
- proportions → real dimensions;
- head motion/retraction envelope;
- arm motion/stow envelope;
- wheel/tuck envelope;
- battery/service envelope;
- internal zones;
- hard interfaces.

## Phase C — Component Verification
Review one component at a time.

For each:
- role;
- real datasheet;
- dimensions;
- voltage/current;
- interfaces;
- thermal/mechanical constraints;
- availability/cost;
- alternatives;
- KEEP / REPLACE / UNDECIDED.

## Phase D — Electrical Architecture
Only after component direction is sufficiently stable.

- power tree;
- charging;
- battery protection;
- current budget;
- buses/interfaces;
- controller resource budget;
- pinout;
- connectors;
- safety.

## Phase E — Digital Prototype / Simulation
Before full purchase.

- CAD fit;
- kinematic checks;
- arm/retraction collision checks;
- center of gravity estimates;
- electrical simulation where useful;
- firmware mocks;
- laptop brain protocol;
- virtual command/state testing.

## Phase F — Breadboard / Bench Bring-up

- power;
- controller;
- communications;
- one actuator at a time;
- sensors;
- audio;
- display/camera;
- integrated safety.

## Phase G — Mechanical Prototype

- print/build;
- assemble modules;
- validate motion;
- iterate clearances;
- verify serviceability.

## Phase H — AI Brain Integration

- wireless protocol;
- speech;
- vision;
- memory/database;
- behavior;
- safe intent layer.

## Phase I — Used Phone Migration

After laptop-based behavior/database is stable:
- edge compute feasibility;
- local database sync;
- phone↔robot protocol;
- offline behavior;
- cloud fallback;
- power/thermal strategy.


## Immediate pre-purchase risk-retirement step
Run the minimum-parts bench campaign in `PRE_PURCHASE_GATE.md` before ordering batches. Prioritize actuator commissioning/thermal tests, cliff surface tests, head cable loop, wheel traction/braking, battery transients, magnet pull/thermal, and belly optical coupons.
