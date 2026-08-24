# Open-source / Pre-purchase Engineering Loop

This Lite branch keeps the project rule used for Reachy, LeRobot, OpenArm, Stretch, MoveIt and Nav2 studies: before buying or freezing hardware, inspect mature open-source implementations, documentation, issue trackers and failure reports for the same class of problem.

Applied fixes in this revision:
- **Integrated electronics instead of many loose modules:** reduces wiring/pin conflicts and gives a documented, repeatable starting platform.
- **Removable rear covers:** software prototypes need repeated access; sealed/welded service panels are avoided.
- **RF window in aluminum head:** a metal enclosure can compromise Wi-Fi/BLE; RF was treated as a mechanical requirement, not an afterthought.
- **No exact camera/mic bracket freeze before parts:** optics and acoustic alignment stay adjustable.
- **Modular base/head interfaces:** this stationary shell can become a software/electronics upper-body test platform after the mobile base and motorized head are developed.
- **USB-first:** avoids buying a battery before measured current/runtime requirements exist.
- **No motion safety complexity in Lite:** because the robot is stationary, cliff sensors, motor drivers and STM32 motion safety are deferred rather than simulated badly.
