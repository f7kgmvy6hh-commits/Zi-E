# 2026-08-31 — Final bench-evidence delta before commit

## HW-008 WS2812-family breakout

- Physical presence confirmed.
- Input side identified as `5V / DI / GND`; output side as `5V / DO / GND`.
- A series resistor measured approximately 0.33 kΩ / 330 Ω with the multimeter.
- Temporary wiring was ESP32 3V3 to the breakout 5V/VCC pad, ESP32 GND to GND,
  temporary GPIO7 through the approximately 330 Ω resistor to DI, and DO unconnected.
- The visible RED, GREEN, BLUE, OFF sequence passed. DI, the RGB channels, and this
  bounded 3.3 V bench arrangement therefore have an independent functional PASS.
- This result does **not** verify final 5 V operation, system wiring, production GPIO,
  or integrated display/RGB commissioning. GPIO7 is temporary bench wiring only. The
  HW678 pin marked 5Vin remains unverified as a 5 V output and must not be used as the
  assumed final source.

## Unmapped capacitive touch module investigation

No repository evidence explicitly maps this physical module to a purchased inventory
ID, so no ID is assigned and it is not added to the project-evidence overlay.

Conservative state: `PRESENT / BENCH_IN_PROGRESS / FUNCTION_NOT_VERIFIED`.

- Labels observed: `GND`, `I/O`, `VCC`, plus unchanged configuration pads A and B.
- It appears TTP223-style/compatible, but exact silicon is not formally verified.
- It was powered from ESP32 3.3 V. Idle I/O measured approximately 3 V and remained
  stable around 3 V during the observation interval while disconnected from ESP32 GPIO.
- Touch/release behavior was seen using temporary GPIO8 and later GPIO7, along with
  false/near-field triggers. Filtering/debounce experiments were inconclusive.
- Reliable physical touch discrimination is not verified. GPIO7/GPIO8 are temporary
  bench pins only; neither is a production assignment. A/B pads remain unchanged.

No flashing, integrated power authorization, or physical commissioning authority is
created by this evidence.
