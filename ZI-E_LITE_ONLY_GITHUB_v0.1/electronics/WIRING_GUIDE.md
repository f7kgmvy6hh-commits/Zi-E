# ZI-E Lite v0.1 Wiring Guide

## Main board
Waveshare ESP32-S3-Touch-LCD-3.5B. The display, capacitive touch, ESP32-S3R8, 8 MB PSRAM, 16 MB flash, IMU, RTC, microphone, audio codec and power management are already on the board.

## Connections
### 1. Camera
OV5640 module -> board camera FPC connector. Follow the connector orientation shown by Waveshare; do not reverse the flex cable. No breadboard jumper wires are used for the camera bus.

### 2. Speaker
Included 6 ohm / 1 W speaker -> MX1.25 speaker connector on the board. Speaker polarity is not critical for a single speaker. Route the two wires through the fixed neck into the lower body speaker chamber.

### 3. RGB emotion LED
Use a common-cathode diffused RGB LED.
- GPIO39 -> 330 ohm -> R anode
- GPIO40 -> 330 ohm -> G anode
- GPIO41 -> 330 ohm -> B anode
- LED common cathode -> GND
Use PWM; begin with low duty cycle. If a particular LED is too dim, resistor values can be tuned after the optical test. Do not exceed the LED/GPIO current ratings.

Header reference from the board pinout: GPIO39, GPIO40, GPIO41 and multiple GND pins are exposed on the 2.54 mm header.

### 4. Power during software development
Laptop/USB supply -> board USB-C. This is the default for v0.1. Do not add the battery until the software and current draw are known.

### 5. Optional battery later
Protected 1S 3.7 V LiPo -> board MX1.25 battery header. Verify connector polarity with the Waveshare schematic/board marking before plugging it in. Never connect a 2S/3S battery to this connector. Never connect the LiPo to the board 5V pin.

### 6. Optional external antenna
If the closed aluminum head reduces Wi-Fi/BLE range, enable the board's IPEX antenna path using the manufacturer procedure and attach a 2.4 GHz flex antenna behind the non-metallic RF window.

## Cable bundle through the neck
Only these cables need to leave the head in the Lite build:
- 2 speaker wires
- 4 RGB wires (R/G/B/GND) if LED is in body
- 2 battery wires if battery is in body
- optional USB-C extension/cable

Keep the camera FPC entirely inside the head.
