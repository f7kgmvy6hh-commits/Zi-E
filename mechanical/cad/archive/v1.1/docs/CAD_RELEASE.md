# ZI-E CAD v1.1 — Prototype Release

## Release meaning

This is the first complete, parameterized mechanical/package CAD of the current ZI-E architecture. It is complete enough to print fit-test parts, simulate state envelopes, build subsystem rigs, and create the electronics PCB around real keep-outs.

It is **not yet a production freeze** because several dimensions depend on real-part measurement and physical tests. That distinction is deliberate engineering practice, not unfinished bookkeeping.

## Coordinate convention
- X = left/right
- Y = front (-) / rear (+)
- Z = up
- units = mm

## Generated system states
- `ZI-E_v1_1_ACTIVE.step`
- `ZI-E_v1_1_SLEEP.step`
- `ZI-E_v1_1_SHUTDOWN.step`
- `ZI-E_v1_1_CUTAWAY.step`
- matching glTF viewer exports

## Current generated envelopes

The automated assembly bounding-box output is stored in `data/assembly_bounding_boxes.json`. Current nominal design intent is approximately:
- Active height: ~282 mm.
- Main body/base envelope: ~184 × 142 mm before external wheel/arm envelope.
- Active full pose envelope: ~248 mm wide and ~174 mm front/rear including relaxed hands.
- Protected shutdown envelope: ~220 mm wide × ~151 mm deep × ~182 mm high.

These values are *outputs* of the component packing model, not aesthetic dimensions chosen first.

## Head
- 3.5 in landscape IPS-touch display envelope: 84.52 × 55.26 × 4 mm.
- OV5640 AF camera envelope reserved in a top-center sensor brow.
- Dual MEMS mic port reservations symmetric around the camera.
- Pan: SC09, 1:1 belt, structural hollow bearing ring.
- Tilt: SC09, ~2:1 belt, dual-side bearing yoke, COM-centered.
- Lift: N20 encoder motor + T5×2 lead screw + one precision rail/two carriage reference + secondary anti-rotation guide provision.
- Active / Sleep / Shutdown are generated from the same model.
- Shutdown top is flush with the body top plane; no latch is used.
- Three structural shutdown seats are included.
- Hollow pan center and dynamic cable keep-out are reserved.

## Body / structure
- Soft tapered old-ZI-E visual direction, not the later industrial humanoid poster direction.
- Cosmetic shell is non-load-bearing.
- Lower structural chassis + left/right shoulder towers + rear bridge surround a protected central Head Bay.
- Side arm-stow pockets are cut through the cosmetic shell, with structure behind them.
- Rear service cover has no essential wiring attached.
- Separate lower-rear battery access door.
- Hidden lower-front speaker chamber/vents.
- ESP32 antenna keep-out reserved at upper rear nonmetallic shell.

## Mobility
- **Two powered wheels only**, differential drive.
- 25D/54L encoder gearmotor class drives an independently bearing-supported axle through a short belt.
- Manual wheel tuck is modeled about an upper-inner longitudinal hinge; stowed tires clear the desk and the robot rests on four fixed feet.
- Hard-stop/lock geometry is reserved.
- One hidden front-center POM ball caster is the third active contact point.
- Passive front/rear anti-tip skids are modeled.

## Desk-edge / collision safety
- Four fixed downward VL53L4CD-class optical windows near the four base corners.
- Four horizontal ToF reservations for local obstacle sensing.
- Four compliant bumper segments as a sensing-independent physical fallback.
- Downward sensor layout remains a physical-test freeze gate for glass, black/glossy surfaces, mirror, sunlight and contamination.

## Battery / electronics package
- Low central 3S2P M35A cassette; six cells modeled at Molicel maximum cell diameter/height.
- Rear-extracting cassette rails, front hard stop and latch/handle provision.
- Rear service-board envelopes for multimedia, safety and power modules.
- BQ7791500-class pack-protection architecture is reserved electrically; shunt/FET/fuse values remain measurement gates.
- Separate power and signal/service zones maintained.

## Arms
Per arm:
1. two bearing-supported shoulder axes, STS3215-class actuators near the body;
2. proximal HL2915 elbow actuator with short belt to the elbow axle;
3. keyed one-stage telescopic forearm;
4. N20 + T5×2 extension drive with end-sensor/thrust-bearing provisions;
5. limited forearm roll, SC09;
6. wrist pitch, SC09 with reduction;
7. manual quick-swap tool interface.

The CAD contains Active, Reach, Desk and Stow kinematic key poses for validation. The current shell pockets clear the defined distal-arm geometry in all four key poses.

## Quick-swap tools
- 3-lug bayonet concept.
- independent pull/twist safety collar architecture.
- asymmetric key.
- 8-contact pattern: 2× V+, 2× GND, differential data A/B, Tool ID, AUX.
- electrical contacts carry no structural load.
- gripper and magnetic-tool envelope prototypes included.

## Manufacturing assumptions
Prototype-oriented starting values are defined in `data/parameters.json` and are intentionally adjustable. The authoritative model is parameterized so printer/material calibration does not require manually redrawing the robot.

## Automated validation result
See `VALIDATION_REPORT.md` and `data/validation_matrix.csv`.

One load case is intentionally marked restricted: **both arms fully forward with 250 g payload on each hand** reduces the calculated static primary-support margin below the normal-driving target. The design response is not to hide that result: firmware must lock/slow the base or reduce simultaneous payload/reach in that state.

## Production-freeze gates
The following must be completed before “final manufacturing CAD” is claimed:
1. Measure the real display and obtain exact FPC/touch-controller documentation.
2. Print fit coupons and calibrate moving/static/insert clearances for the actual printer/material.
3. Weigh the assembled head and measure its COM; then freeze Tilt reduction/counterbalance.
4. Build one dummy arm and perform horizontal-hold, transient, backlash and thermal tests using rated—not stall—torque.
5. Cycle-test head lift, telescope, wheel fold and tool bayonet for wear/jam/cable fatigue.
6. Validate cliff sensing on the required surface matrix.
7. Measure real robot current waveforms; then freeze battery shunt/FET/fuse/eFuse settings.
8. Run speaker/microphone A/B acoustic tests with motors/display active.
9. Run thermal tests with the shell installed and in shutdown.
10. Replace low-confidence mass estimates with measured part masses and rerun stability.
