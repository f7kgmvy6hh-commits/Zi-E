# Research Sources

1. Waveshare ESP32-S3-Touch-LCD-3.5B product/documentation
   - https://www.waveshare.com/esp32-s3-touch-lcd-3.5b.htm
   - https://docs.waveshare.com/ESP32-S3-Touch-LCD-3.5B
   Key design inputs: 3.5-inch 320x480 capacitive IPS, ESP32-S3R8, 8 MB PSRAM, 16 MB Flash, QMI8658 IMU, PCF85063 RTC, ES8311 audio codec, onboard microphone, AXP2101 power/battery management, camera interface supporting OV5640/OV2640, battery/speaker headers, 92.44 x 61.00 mm bare-board mechanical envelope.

2. Protocase Electronic Enclosure Design 101
   - https://www.protocase.com/blog/2017/02/16/electronic-enclosure-design-101/
   Design input: 5052 aluminum is well suited to sheet-metal enclosures and is easily welded/machined; it bends more reliably than 6061 for enclosure work.

3. Protocase aluminum thickness/tolerance and bend-radius references
   - https://www.protocase.com/resources/tolerances/aluminum.php/1000
   - https://www.protocase.com/resources/bend-radii/
   Design input: real sheet thickness and fabrication tolerances must be accounted for; do not assume perfect nominal stock or zero weld distortion.

4. Adafruit OV5640 camera guide
   - https://learn.adafruit.com/adafruit-ov5640-camera-breakout
   Design input: parallel camera signals are sensitive to sloppy breadboard wiring; keep the FPC/interface short and use the intended connector.
