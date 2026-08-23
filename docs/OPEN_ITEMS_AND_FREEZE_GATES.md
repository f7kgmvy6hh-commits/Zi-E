# Open Items / Freeze Gates

The project may continue, but the following cannot be silently treated as final:

1. Display: confirm exact selected FPC pinout, touch controller, connector, backlight and initialization details.
2. Head: measure complete head assembly mass/COM; re-size Tilt ratio and lift/tilt counterbalance from measurements.
3. Arms: build weighted dummy arm; verify rated-torque margins, backlash, stall/jam behavior, belt tension, thermal rise and endurance.
4. Telescope: verify lead-screw/nut wear, thrust bearing, stroke, wobble and dynamic cable life.
5. Wheels: choose final tire diameter/width/material and pulley ratio from traction, noise, scrub and braking tests.
6. Cliff: validate black/glass/mirror/sunlight/dirty-sensor cases; add different-principle mechanical fallback if needed.
7. Magnet: characterize pull force against representative parts and air gaps; characterize continuous/limited duty temperature.
8. Battery/BMS: measure actual worst normal/blocked currents; then freeze shunt, MOSFETs, fuse and wire/connector sizes.
9. Audio: A/B microphone ports and speaker chamber; verify AEC with motors and speaker simultaneously active.
10. Wiring: validate bend radii, strain relief, service loops, EMI and wear cycle counts.
11. Shell: apply final industrial-design surfaces only after all service/collision/thermal/sensor keep-outs pass.
12. Charging: V1 uses external 12.6V CC/CV; integrated USB-C PD is a future revision, not a hidden requirement.


## Belly Matrix / hidden HMI
- Optical dead-front coupon: off-state concealment vs lit readability.
- Sunlight/daylight visibility and color shift.
- Diffuser/baffle distance and light-leak test.
- Full-white/current/temperature stress.
- Dedicated non-safety bus and fail-off power gating.

## Commissioning and fault containment
- Actuator scan/model/baud/ID/direction/range before normal torque.
- Bus topology / cable fault injection: loss of one actuator/cable must degrade predictably.
- Command protocol must distinguish accepted from physically executed.
- Command arbiter prevents multiple simultaneous setpoint writers.
- Diagnostic bundle/system-check must exist before full integrated prototype testing.


## Head laser rangefinder freeze gate
- Verify VL53L1X prototype carrier and final sensor brow packaging.
- Select/measure IR-transmissive cover material around 940 nm.
- Minimize air gap/tilt and perform offset + crosstalk calibration.
- Calibrate camera-to-rangefinder optical axes and document the ranging point.
- Test known distances, dark/daylight, black/glossy/glass targets and contamination.
- Do not promote to BUY-BATCH until results are saved under `tests/results/`.
