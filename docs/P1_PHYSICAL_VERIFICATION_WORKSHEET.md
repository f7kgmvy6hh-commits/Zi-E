# P1 Arrival-Day Physical Verification Worksheet

Do not prefill measured fields. Attach inventory ID, dated photo, instrument, method,
operator and source/datasheet to every result. A CAD estimate is not a measurement.

## Part identity and fit

| Check | Inventory ID | Documented value/source | Measured value | Photo/ref | Pass/fail/decision |
|---|---|---|---|---|---|
| Exact manufacturer/model/variant/revision/label/serial | | | | | |
| Board/body length x width x height | | | | | |
| Mounting-hole pattern, diameter and datum | | | | | |
| Connector type, cavity/pin count, location and orientation | | | | | |
| Cable/backshell exit and minimum observed bend radius | | | | | |
| Component/assembly mass | | | | | |
| CAD envelope/interference/service-removal fit | | | | | |

## Power, actuators and mobility

| Check | Inventory ID/test condition | Documented value | Measured value | Instrument/log | Result |
|---|---|---|---|---|---|
| Battery/cell/pack exact dimensions and mass | | | | | |
| Battery provenance/protection/connector/polarity | | | | | |
| Rail voltage unloaded/loaded/inrush | | | | | |
| Actuator exact identity/model/voltage class | | | | | |
| Bus/baud/ID/feedback discovery with motion disabled | | | | | |
| Actuator travel, zero, direction, hard/end state | | | | | |
| Motor/servo idle, no-load and bounded-load current | | | | | |
| Bounded-load temperature versus time/ambient | | | | | |
| Wheel diameter, width, runout and tread | | | | | |
| Wheel traction/braking/turn scrub by surface | | | | | |
| Caster height/friction and support contact | | | | | |

## Optics, sensors, audio and moving cables

| Check | Inventory ID/test condition | Documented value | Measured/observed value | Evidence | Result |
|---|---|---|---|---|---|
| Sensor/camera field of view and occlusion | | | | | |
| VL53L1X window transmission/crosstalk by material/air gap | | | | | |
| Camera lens/focus/FPC clearance through head range | | | | | |
| Display IC/interface/FPC/touch and active area | | | | | |
| Magnet pull vs material/air gap and temperature | | | | | |
| Speaker/microphone chamber/port/cable clearance | | | | | |
| Head/arm cable loop bend, pinch and full-range clearance | | | | | |
| Retraction/flex cycle count and observed cable wear | | | | | |
| Cliff sensing: white/matte black/glossy/metal/mirror/glass/cloth/edge/sunlight/dirt | | | | | |

## Mass and center-of-mass inputs

| Module/configuration | Mass measured | Datum | Balance/COM observation | Uncertainty/method | Evidence |
|---|---|---|---|---|---|
| Base/chassis | | | | | |
| Battery/pack | | | | | |
| Torso/electronics | | | | | |
| Head assembled | | | | | |
| Left/right arm assembled | | | | | |
| Tools/payloads | | | | | |
| Complete active/sleep/shutdown robot | | | | | |

After data entry, update the inventory row, Device Identity, Hardware Profile candidate,
CAD parameter provenance and risk/gate result together. Conflicts require Product Owner
review; never silently substitute a physically different part.

## Physical no-go sign-off

- [ ] Exact battery protection and safe source/protection path verified.
- [ ] Every powered actuator identity/voltage/direction/range/feedback verified.
- [ ] Current/thermal limits derived from measurements and reviewed.
- [ ] Cliff/bumper and required local safety responses physically pass.
- [ ] Moving cables/strain relief clear full constrained range.
- [ ] Range window/camera/display interfaces pass with received parts.
- [ ] Physical safe-stop delivery and actual-state confirmation observed.
- [ ] Restrained commissioning completed and signed.

If any box is blank or failed: no autonomous motion and no unrestrained driving.

