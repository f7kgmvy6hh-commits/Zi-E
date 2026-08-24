# Aluminum Build Guide

## Material and fabrication strategy
- Preferred: 5052-H32 aluminum, 1.5 mm shell; 2.0 mm stationary base.
- Weld only the structural/cosmetic shell seams. Keep the body rear panel and head rear panel removable with screws.
- Add small internal aluminum-angle tabs/rails for the removable covers.
- Never weld with electronics, camera, speaker or battery installed. Remove all metal chips before installing electronics.

## One-shot fabrication loop
1. Print the 1:1 PDF at **Actual Size / 100%**. Measure the 50 mm calibration line. If it is not exactly 50 mm, stop.
2. Tape the paper to the aluminum and center-punch every marked pilot point.
3. Cut the shell pieces slightly proud if cutting by hand; final-scribe and trim during dry fit. Handmade TIG-welded sheet metal will not hold sub-0.5 mm enclosure tolerances without a jig.
4. Dry-clamp the whole body. Check top/bottom diagonals and symmetry.
5. Tack weld opposite corners in alternating order. Do not run a long continuous bead on one side first; heat distortion will move the shell.
6. Recheck the display opening and removable rear-cover fit after tacking.
7. Finish with short alternating welds/stitches. Let the part cool between sections.
8. Grind cosmetic outside seams only as much as necessary. Keep internal corner material where it adds stiffness.
9. Fit rubber feet and the removable stationary base plate.
10. Install insulating board spacers, foam gasket around the display, RF window, microphone vent foam and speaker only after all metalwork is done.

## Electronics insulation
- Keep at least ~2-3 mm clearance between PCB solder joints and aluminum.
- Use M2 standoffs plus insulating washers or a thin FR4/Kapton barrier where needed.
- No loose wire should be able to rub on a sharp aluminum edge; use grommets and edge trim.

## RF window
The ESP32 antenna must not be fully surrounded by aluminum. Fit a 35 x 20 mm polycarbonate/ABS/acrylic window in the marked rear/right head zone. Keep metal and battery wiring away from that zone. If Wi-Fi/BLE remains weak, use the board IPEX external antenna option.

## Serviceability
The rear body and rear head covers are intentionally not welded. The software prototype will be opened many times. This is a feature, not a cosmetic compromise.
