# XiaoZhi ESP32 Adoption Audit

## Source and method

Repository: https://github.com/78/xiaozhi-esp32  
Pinned commit: `374a5ccf95c2ed513dbad9ca204adbac75062dce`  
License: MIT (copyright 2025 Shenzhen Xinzhi Future Technology Co., Ltd. and
Project Contributors). No files or assets were copied in this pass.

The sandbox blocked a direct HTTPS clone, so the audit used GitHub's immutable commit
tree/raw views plus official component documentation. Moving `main` was not used as
source truth. A future code adoption must re-fetch this SHA and compare the exact file
before copying. Dependency and asset licenses remain separate; see
`THIRD_PARTY_NOTICES.md`.

Decision meanings: **ADOPT** means adopt the idea/contract; **ADAPT** means a later
small implementation may be derived behind Zi-E boundaries with notice; **REFERENCE_ONLY**
means learn from behavior but copy nothing; **REJECT** means incompatible with Zi-E;
**DEFER** means hardware/evidence/build prerequisites are absent.

## Component-level decisions

| Donor source at pinned SHA | Purpose | Decision / Zi-E destination | Provenance and risk controls |
|---|---|---|---|
| `CMakeLists.txt`, `main/CMakeLists.txt`, `main/idf_component.yml`, `sdkconfig.defaults*` | ESP-IDF organization, trimmed build, managed components | **ADOPT** project/binding pattern in `firmware/targets/esp32`; **REFERENCE_ONLY** exact flags and broad matrix | Donor MIT, dependencies separate. Zi-E does not copy its board matrix or force its IDF version. |
| `main/boards/**`, `docs/custom-board.md` | Per-board config and initialization isolation | **ADOPT** one verified board identity per binding in `firmware/board_profiles`; HW-002 binding remains disabled | Board examples are not evidence. No donor/seller pinout enters production automatically. |
| `main/boards/common/wifi_board.*`, `78/esp-wifi-connect` dependency | Wi-Fi lifecycle/provisioning | **ADAPT** reconnect/provisioning behavior behind future Presence Runtime network service | Credentials, captive setup, reset behavior, and package license/security need review. Network never grants motion. |
| `main/protocols/protocol.*`, `websocket_protocol.*`, `docs/websocket.md` | Persistent control/audio connection, binary Opus, reconnect state | **ADAPT** failure handling behind `HOST_ESP32_INTERFACE_V1.md` | XiaoZhi cloud schema/session assumptions rejected; Zi-E authentication and command identities mandatory. |
| `main/protocols/mqtt_protocol.*` | MQTT+UDP alternative | **REFERENCE_ONLY** for validation/bounds lessons | Not selected; additional transport expands attack/state space. |
| `main/audio/**`, especially `audio_service.*`, codec/processors and Opus paths | Capture/playback, buffering, resampling, codec and DSP coordination | **ADAPT** small proven scheduling/glue patterns after hardware verification | Use official codec/DSP dependencies; concurrency, bounds, and half/full-duplex hardware assumptions require bench tests. |
| `main/idf_component.yml`: `espressif/esp_audio_codec`, `esp_codec_dev`, `esp_audio_effects` | Audio codecs/effects | **ADOPT upstream**, not donor copies | Version/license pinned during implementation; codec and I2S board binding disabled until evidence. |
| `main/idf_component.yml`: `espressif/esp-sr` | Wake, AFE/AEC/NR capabilities | **ADOPT upstream** for future local wake; **DEFER** configuration | Hardware/channel/reference layout and memory budget unverified; safety grammar remains separate. |
| Opus use within `main/audio/**` and protocol code | Compressed bidirectional audio | **ADAPT** framing/buffering patterns; upstream codec dependency to be chosen explicitly | Host transport independent of XiaoZhi server. Packet sizes/rates remain unfrozen. |
| `main/boards/common/esp32_camera.*` and board camera bindings | Camera abstraction, JPEG/capture/failure handling | **REFERENCE_ONLY/ADAPT** after HW-002 arrival | Prefer official `espressif/esp32-camera`; consider `esp_video` only if verified device/IDF needs it. Never copy Waveshare pins. |
| `main/display/**`, board display/touch bindings | Display abstraction and LVGL integration | **ADAPT** lifecycle patterns behind Zi-E Face Engine | Prefer `lvgl/lvgl` and `esp_lvgl_port`; controller, VCC/logic, resolution, memory remain unverified. Optional touch absence must degrade, not crash. |
| `main/assets.*`, `main/assets/**`, emoji/font/image dependencies | Asset loading and presentation content | **REFERENCE_ONLY** storage/loading concepts; XiaoZhi assets **REJECT** as Zi-E face architecture | Individual asset licenses/provenance unknown until audited. Zi-E uses composable data-only packs. |
| `main/device_state*`, `main/application.*` | Explicit device-state transitions | **ADAPT** useful non-motion presence/reconnect states | Do not import global/cloud authority coupling or confuse listening/connected with robot readiness. |
| battery/power helpers under board/common and `adc_battery_estimation` dependency | Battery/status display | **DEFER** | HW-002/power topology and ADC scaling unverified. Status cannot authorize power. |
| `main/settings.*` | NVS-backed settings namespaces | **REFERENCE_ONLY** | Future values must integrate Zi-E transactional configuration, secrets, package identity, and rollback. |
| `main/system_info.*`, logging in `application.*` | Diagnostics/build/device information | **ADAPT** bounded sanitized telemetry | Never use COM/IP/discovery order as identity; avoid secrets and unbounded logs. |
| `main/mcp_server.*`, `docs/mcp*` | Tool schema/discovery and device tools | **ADAPT** schema/discovery concepts for diagnostics/presence only | Generic GPIO/PWM/servo/motor tools **REJECT**. Motion must traverse Host semantic authority and STM32. |
| donor IoT/device tool registrations in board code | Speaker, LED, GPIO, servo controls | **REJECT** as public physical-control architecture | Allowed future tools: status, verified camera capture, face/RGB/audio settings, diagnostics. No direct motion. |
| `main/ota.*` | Version check/download/update/reboot | **REFERENCE_ONLY** failure handling; arbitrary URL update **REJECT** | Future firmware path must use Zi-E package verification, trust, compatibility, rollback, quarantine, and explicit target identity. |
| `main/led/**` and `espressif/led_strip` | RGB status/effects | **ADOPT upstream** after HW-008 identity/logic verification | Effect semantics only; pins/electrical limits remain board-specific. |

## ESP-IDF support decision

Zi-E supports no production ESP-IDF build yet. The pinned donor manifest declares
`idf >=5.5.2` and its pinned README prefers stable 6.0.x, but that does not prove
HW-002 compatibility. Candidate evaluation range is **ESP-IDF 5.5.2 through stable
6.0.x**, narrowed only after HW-002 identity, official component compatibility, and a
reproducible no-flash build matrix. ESP32-S3 support, official component versions,
and migration cost decide the selected pinned release; moving donor main does not.

## Failure lessons retained

- Camera APIs/components can become incompatible across IDF/component versions;
  compile the verified camera binding against a locked matrix and fail availability
  without destabilizing presence runtime.
- Missing/incorrect camera device nodes or pixel-format negotiation must return a
  typed unavailable fault, bound allocation, and preserve audio/face operation.
- Optional touch absence must disable touch only, not abort display/runtime startup.
- Audio capture/playback/reconnect are concurrent state machines; bound queues,
  cancel stale sessions, and test second-utterance/reconnect recovery.
- Weak-network disconnect can strand listening state; transport loss must terminate
  the audio session and transition deterministically without granting robot readiness.
- Validate packet type/length before decode and bound every buffer/reassembly path.
- OTA failure must retain the verified known-good image and cannot accept arbitrary URL
  authority.

These findings are recorded in `OPEN_SOURCE_HARVEST_AUDIT.md` and `RISK_REGISTER.md`.

## Local modifications / adopted artifacts

No donor implementation or assets were copied. Zi-E-authored outcomes are the six V1
contracts, `app/presence/face_engine.py`, `app/voice/safety.py`, tests, and provenance
records. XiaoZhi remains replaceable without changing Zi-E's semantic architecture.
