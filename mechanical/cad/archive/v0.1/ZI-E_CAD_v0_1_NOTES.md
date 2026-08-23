# ZI-E CAD v0.1 — Packaging / Architecture Model

## Status
This is the first parametric CAD packaging model built from the current ZI-E design decisions. It is **not a manufacturing-final CAD**. External dimensions remain provisional until collision, stability, mass/COM, cable bend, thermal, and physical bench tests are completed.

## Design direction preserved
- Compact desktop robot; original soft futuristic ZI-E identity (not industrial humanoid).
- One wide landscape digital face.
- Hidden top-center camera brow.
- Head Pan + Tilt + vertical retract.
- Two powered side wheels only + one hidden passive ball caster.
- Differential drive; no Mecanum/4-wheel drive.
- 2-DOF shoulder, elbow, telescopic forearm, limited roll, wrist pitch, quick-swap tool.
- Low central 3S2P battery cassette.
- Rear service electronics zone.
- Speaker low/front; microphones in head.
- Four downward cliff zones + four horizontal proximity reservations + compliant bumper fallback.
- Structural shoulder towers around a central Head Bay; cosmetic shell is not load-bearing.

## CAD v0.1 provisional envelopes
| Envelope | v0.1 value |
|---|---:|
| Base shell | 188 × 148 × 62 mm |
| Torso shell | 174 × 132 × 112 mm |
| Head shell | 118 × 34 × 80 mm |
| Display candidate | 84.52 × 55.26 × 4 mm |
| Wheel diameter | 62 mm |
| Active overall height | ~290 mm |
| Overall width across wheels | ~210 mm |

These numbers are outputs of a first packing pass, **not frozen product dimensions**.

## Real-component / selected envelopes represented
- Display candidate: 3.5 in IPS capacitive-touch ILI9488 class, landscape.
- Camera: OV5640 autofocus class; final head uses a compact FPC module, not a large prototype breakout.
- Main MCU: ESP32-S3-WROOM-1-N16R8.
- Safety/motion MCU: STM32G0B1RET6.
- IMU: LSM6DSOX.
- Shoulder: STS3215-C018, 45.2 × 24.7 × 35 mm class.
- Elbow: HL2915 class.
- Roll / wrist / gripper and head pan/tilt: SC09 class.
- Head lift and telescopic forearm: N20 encoder gearmotor + T5 lead screw.
- Battery: 3S2P Molicel M35A-class 18650 cassette.
- Wheels: 25 mm 12 V encoder gearmotor class driving an independently bearing-supported wheel axle.
- Speaker: PUI 40 × 28.5 × ~12 mm class.
- Cliff/proximity: VL53L4CD-class sensor reservations.

## Critical unresolved/freeze gates
1. Verify exact display FPC pinout and capacitive-touch controller before PCB freeze.
2. Measure real head mass and center of mass before Tilt ratio freeze.
3. Build a dummy arm and validate shoulder/elbow thermal/torque margin before arm geometry freeze.
4. Validate cliff sensors on matte black, glossy black, mirror, glass, tablecloth, sunlight, dirty/blocked sensors.
5. Test wheel diameter/tread, caster load distribution, braking distance and turn scrub.
6. Test P20/15-class magnetic pickup force and coil temperature on real objects.
7. Freeze BQ77915 variant, shunt, FETs, fuse and wiring after full-system current measurements.
8. Run cable swept-volume and minimum-bend-radius checks through Tilt, Pan, lift, telescope and wrist roll.
9. Run thermal airflow tests with shell installed and in protected shutdown.
10. Verify privacy/motion-kill hardware paths independently from firmware.

## Next CAD pass
- Replace envelope geometry with component-specific mounting models.
- Head-bay swept-volume model (Active/Sleep/Shutdown).
- Full arm kinematics and collision envelopes.
- Wheel folding pod + lock/hard-stop geometry.
- Battery cassette rails/latch/service extraction path.
- Rear service shell and internal mounting grid.
- Preliminary mass properties and stability envelope.
