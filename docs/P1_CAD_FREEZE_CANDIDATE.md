# Prototype P1 CAD Freeze Candidate

Status: architecture/envelope study ready for measured-part reconciliation. Current
source is `mechanical/cad/current` (v0.3). Archived CAD is non-authoritative reference.
Concept/reference images are not dimensionally authoritative.

## Fixed now — architectural only

- Hybrid desktop-companion form, active/sleep/protected-shutdown states.
- Central retracting head concept with pan/tilt/lift and protected docking.
- Landscape digital face, centered camera region, separate head range sensor.
- Two-wheel differential base, passive third contact, folding/tucking wheel concept,
  shutdown feet, two functional arms, adaptive gripper and quick-swap concept.
- Separate ESP32 multimedia and STM32 protected-safety/motion zones.
- Rear/battery service access direction, protected power-domain philosophy, belly
  optical keepout, wiring corridors and service-loop intent.
- Parametric source, named components, state exports, FMEA/traceability/check scripts.

“Fixed” does not freeze the numerical v0.3 parameters against received-part evidence.

## Parametric until part arrival

| Area | Current CAD intent | Required reconciliation |
|---|---|---|
| Overall envelope/body/head | v0.3 modeled envelope and four states | Printed process, received modules, cable loops, measured mass/COM and service access |
| Display | Landscape bezel around candidate envelope | Exact glass/PCB/FPC/touch IC, adhesive/retention stack, active area |
| Camera | Centered brow envelope | Exact board/lens/FPC, focus clearance, connector and FOV |
| Range sensor | VL53L1X prototype carrier plus final-sensor reservation | Received form, IR window material/thickness/air gap, alignment/crosstalk |
| Arms/gripper/tools | Link shells and candidate actuator envelopes | Servo bodies/splines/horns, pulleys/belts, link mass, tendon/tool geometry |
| Wheels/caster | Candidate wheel/motor/caster envelopes and pod kinematics | Tire/motor/shaft/mount/caster dimensions, traction and lock loads |
| Battery/service volume | 3S2P class cassette and doors | Actual cells/holders/BMS/fuse/connector/wire bend/service clearances |
| Controller/power zones | ESP32 module and STM32/main/power board reservations | Exact boards, antenna/debug/USB, connectors, heatsinking and mounting holes |
| Wiring corridors/cable loops | Nominal passages and hollow pan route | Harness OD, bend radius, flex class, connector backshells, full-range loop sweep |
| Ventilation/acoustics | Speaker chamber and conceptual vents | Actual heat/audio tests, dust/foreign-object strategy, shell material |
| Fasteners/brackets/strain relief | Nominal M3/insert and bracket geometry | Printer/material coupons, exact fastener stack, inserts, clamps/grommets and loads |
| Sensor windows/collision zones | Modeled key poses and optical openings | Material coupons, full trajectory/cable collision, sensor performance and occlusion |

## Blocked by physical measurement

- Final hole patterns, board standoffs, connector cutouts/orientations and cable exits.
- Final display/camera/range-sensor window and bezel geometry.
- Final battery cassette, pack restraint, power-board mounts and ventilation.
- Final actuator mounts, ratios, hard stops, horn/spline interfaces and moving loops.
- Final wheel/caster contact geometry, tread/fairings, support polygon and braking
  assumptions.
- Final structural clearances, fastener lengths, inserts, wall/fit compensation and
  service-removal paths.
- Final mass/COM, payload/stability, torque/current/thermal and safe-stop-dependent
  geometry. All are `VERIFY_ON_ARRIVAL` plus bench evidence.

## Optional/future

- Custom production belly LED PCB after optical/power coupons.
- Alternative actuator/caster/range-sensor implementations only after documented
  switch conditions and Product Owner approval.
- Integrated USB-C charging, phone migration, production cosmetic surfacing, and any
  production manufacturing optimization.

## CAD evidence interpretation

The v0.3 automated report currently records 38 passing checks and one deliberate
restricted case. This validates internal model consistency at its assumed parameters.
It does not prove printed fit, purchased-part fit, physical mass/COM, torque, current,
thermal behavior, traction, sensor performance, cable life, or safe-stop timing. The
two-arm forward-payload case remains restricted even as a model result; no physical
motion permission follows from it.

