# Phase 2B2 Bus-Load and Timing Analysis

## Purpose and disposition

This analysis evaluates whether the ESP32-S3 to STM32G0B1 Classical CAN/TWAI wire
contract can be frozen. It separates repository-verified facts, protocol-derived
bounds, analysis assumptions, and physical values that remain `TODO / UNVERIFIED`.

**Disposition: Phase 2B2 wire/framing implementation is blocked at its freeze gate.**
The controller pair and Classical CAN link are supported, but the repository lacks the
measured/configured inputs needed to prove bus utilization, worst-case response time,
fragmentation behavior, heartbeat timing, or physical-layer margin. No CAN identifier,
bitrate, payload layout, timeout, integrity mechanism, or retry policy is frozen here.

## Verified facts

- Repository decisions select ESP32-S3-WROOM-1-N16R8 for multimedia/presence,
  STM32G0B1RET6 for safety/motion, and Classical CAN/TWAI between them.
- Espressif documents that ESP32-S3 TWAI is not CAN-FD compatible. Classical data
  frames provide at most 8 data bytes and use 11-bit or 29-bit identifiers.
- ST documents two FDCAN peripherals on STM32G0B1; they can participate in Classical
  CAN when configured accordingly. This does not verify Zi-E pins, clocks, or timing.
- CAN arbitration gives lower numeric identifiers higher priority. Automatic
  retransmission, error counters, error-passive, bus-off, RX queue overflow, arbitration
  loss, and bus recovery are observable controller behaviors.
- CAN's link CRC/error confinement detects many transmission faults but does not
  authenticate a sender and does not provide long-lived replay protection.
- Existing Phase 2A identity is 12 bytes (`source`, `session`, `sequence`). Phase 2B1
  Hello and Heartbeat are each 12 semantic bytes before a wire envelope or integrity
  metadata. None fits in one 8-byte Classical CAN data field without packing fields into
  the identifier or using multiple frames.

## Calculated Classical CAN frame bounds

These bounds use CAN 2.0 field lengths and the rule that stuffing applies from Start of
Frame through the 15-bit CRC sequence. They include the three-bit intermission and
assume a successful data frame with no arbitration delay, retransmission, error frame,
overload frame, software queueing, or ISR/task latency.

For `d` data bytes, where `0 <= d <= 8`:

```text
standard base bits       = 47 + 8d
standard stuffable bits  = 34 + 8d
standard conservative upper bound
                         = 47 + 8d + floor((33 + 8d) / 4)

extended base bits       = 67 + 8d
extended stuffable bits  = 54 + 8d
extended conservative upper bound
                         = 67 + 8d + floor((53 + 8d) / 4)
```

The conservative stuffing term allows one inserted bit per four original bits after
the first. Actual transmitted length depends on identifier, payload, and computed CRC.

| Format and payload | Base bits | Conservative upper bound | Frame time at bitrate `R` |
|---|---:|---:|---|
| Standard, 0 bytes | 47 | 55 | `55 / R` seconds upper bound |
| Standard, 8 bytes | 111 | 135 | `135 / R` seconds upper bound |
| Extended, 0 bytes | 67 | 80 | `80 / R` seconds upper bound |
| Extended, 8 bytes | 131 | 160 | `160 / R` seconds upper bound |

Sensitivity only—not selected Zi-E bitrates:

| Assumed bitrate | Standard 8-byte upper-bound time | Extended 8-byte upper-bound time |
|---:|---:|---:|
| 125 kbit/s | 1080 us | 1280 us |
| 250 kbit/s | 540 us | 640 us |
| 500 kbit/s | 270 us | 320 us |
| 1 Mbit/s | 135 us | 160 us |

The 12-byte semantic Hello or Heartbeat has a lower bound of two frames before adding
fragment indexes, type/version fields, an end-to-end check, or authentication data.
Using two full standard frames gives a conservative serialization bound of `270 / R`
seconds; this is not a final framing proposal and excludes arbitration and errors.

## Required utilization calculation

For each periodic or rate-limited frame class `i`:

```text
C_i = conservative transmitted bits for its selected format and DLC
U_i = frames_per_release_i * C_i / (bitrate * period_i)
U_total = sum(U_i) + explicitly budgeted diagnostic/event traffic
```

Worst-case latency also needs priority-aware response-time analysis. At minimum it must
include one lower-priority non-preemptive blocking frame, interference from every
higher-priority frame that can arrive during the response window, release jitter,
software/ISR queueing, and a separately justified error/retransmission budget. Average
utilization alone cannot prove a safety deadline.

No numeric Zi-E utilization or response-time result is valid yet because the message
inventory, rates, deadlines, bitrate, priorities, and measured implementation latency
do not exist in approved repository evidence.

## TODO / UNVERIFIED inputs required before freeze

Physical and controller timing:

- selected CAN transceiver on each controller and datasheet propagation delays;
- exact bus topology, total cable length, stub lengths, connector count, cable
  impedance, termination placement/value, and node count;
- ESP32-S3 and STM32 CAN clock sources, frequency tolerance across voltage/temperature,
  timing-quanta ranges, sample point, synchronization jump width, and selected bitrate;
- verified ESP32-S3 and STM32 pins plus transceiver standby/fault wiring;
- oscilloscope evidence for edge quality, ringing, common-mode margin, and sampling at
  both temperature and power/noise corners.

Traffic and software timing:

- complete semantic message inventory and direction;
- release period/minimum inter-arrival, burst size, deadline, jitter, and criticality
  for every command, report, confirmed-state, fault, diagnostic, and heartbeat class;
- maximum encoder/fragment count per semantic message;
- arbitration priority/identifier allocation and acceptance-filter plan;
- measured ISR-to-task and task-to-transmit queue latency on both controllers;
- bounded RX/TX queue capacities and overflow policy;
- automatic retransmission versus single-shot policy per message class;
- bus-off detection, local safe-state entry, recovery, and rejoin timing;
- required motion safe-stop deadline and evidence linking it to heartbeat/lease timing.

Integrity and security:

- canonical byte order and schema/version encoding;
- fragmentation transaction identity, ordering, duplicate handling, timeout, and bounded
  reassembly memory;
- end-to-end corruption detection beyond per-frame CAN CRC, if justified;
- authenticated sender/replay design or an explicit documented physical trust model;
- key provisioning/rotation/recovery if authentication is selected.

## Failure analysis and required verification

| Failure | Prevention | Detection | Recovery | Required test |
|---|---|---|---|---|
| Priority starvation or excessive bus load | Schedulable ID/rate budget; bounded diagnostics | Utilization counters, deadline misses, queue high-water marks | Drop/defer noncritical traffic; local safe state on critical deadline loss | Worst-case simultaneous release and diagnostic burst |
| RX queue overflow | Acceptance filters and bounded producer rates | Controller RX-full/overflow alerts | Preserve safety traffic; degrade diagnostics/cosmetics | Slow-consumer and interrupt-delay injection |
| Arbitration/retransmission jitter | Priority-aware response analysis and error budget | Arbitration-lost/error counters | Local lease expiry; bounded recovery | Competing traffic plus injected bit/ACK errors |
| Bus-off or stuck controller | Fault confinement and independent local safety | Error-passive/bus-off alerts and missing liveness | Local safe state; controlled recovery without automatic motion re-arm | Disconnect, short/noise simulator, repeated bus-off recovery |
| Fragment loss/reorder/duplication | Bounded transaction identity and fail-closed reassembly | Sequence/gap/timeout diagnostics | Discard incomplete semantic message; never partially execute | Drop, duplicate, reorder, stale-session, and reboot corpus |
| Spoof/replay | Authentication or justified physical trust boundary | Integrity/session/replay diagnostics | Reject, quarantine/rekey/service as designed | Configured-peer spoof and long-lived replay tests |

## Freeze criteria

Phase 2B2 wire implementation may begin only when:

1. The physical/controller timing inputs above are verified from selected parts and
   schematics or explicitly bounded prototype measurements.
2. The complete message inventory supplies rates, bursts, deadlines, and criticality.
3. Standard versus extended identifiers and priority allocation pass utilization and
   worst-case response-time analysis with declared margin.
4. Fragmentation and bounded reassembly are reviewed against reset, loss, duplication,
   stale-session, and memory-exhaustion failures.
5. Integrity/authentication scope is decided without claiming CAN CRC authenticates a
   peer.
6. Bench tests confirm timing, queue behavior, error handling, and local safe-state
   response under injected faults.

Until then, Phase 2A and Phase 2B1 remain the validated transport-independent boundary.

## Sources and provenance

- Zi-E repository: `docs/ZI-E_MASTER_SPEC.md`, `docs/COMPONENT_MASTER_MATRIX.md`,
  `docs/PHASE2_PLAN.md`, and committed Phase 2A/2B1 code.
- Espressif ESP32-S3 TWAI documentation:
  https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/twai.html
- Espressif TWAI FAQ (ESP32-S3 has no integrated CAN-FD controller):
  https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/peripherals/twai.html
- ST STM32G0B1 product/datasheet and AN5348 FDCAN introduction:
  https://www.st.com/en/microcontrollers-microprocessors/stm32g0b1cb.html
  and https://www.st.com/resource/en/application_note/an5348-introduction-to-fdcan-peripherals-for-stm32-mcus-stmicroelectronics.pdf
- Bosch CAN Specification 2.0, frame fields and bit-stuffing rule:
  https://tech-tools.com/files/can2spec.pdf

No external code or wire format was copied. Vendor documentation and the Bosch
specification inform calculations only; any future imported implementation requires a
separate license/provenance review.
