# AGENTS.md — ZI-E Lite v0.1

Any AI/code agent working in this repository must treat this file plus `CURRENT_STATE.md` and `docs/MASTER_SPEC.md` as the current source of truth.

## Mandatory project rules
1. This branch is **Lite only**: stationary, fixed head, aluminum body, software-first.
2. Do not reintroduce arms, wheels, Pan/Tilt/Lift, laser, cliff sensors, STM32 motion control, large RGB matrix, 3S traction battery, or other Full-version hardware unless the user explicitly starts an upgrade branch.
3. No 3D-printed structural body is assumed. Fabrication is based on aluminum cut templates, removable service panels, screws/rivnuts/tabs, and welded shell seams where appropriate.
4. Preserve the RF window and electrical insulation rules; aluminum is both a structural material and an RF/electrical hazard if treated casually.
5. Prefer the integrated Waveshare ESP32-S3-Touch-LCD-3.5B platform for Lite unless a documented blocker appears.
6. Use USB-C first. Do not assume a battery is present. The optional battery is a protected 1S LiPo and is not the future traction pack.
7. Keep APIs hardware-agnostic so the later Full robot can reuse face/voice/vision/AI software.
8. Before freezing or buying hardware, run: Memory Loop → Research/Open-Source Harvest → Alternatives → FMEA → Integration/Interface → Cost → Test plan.
9. Do not silently change dimensions. Any geometry change must update `design_params.json`, templates/CUT_LIST, relevant CAD/templates, and documentation together.
10. Distinguish verified vendor dimensions from nominal fabrication allowances and from still-unverified part geometry.
