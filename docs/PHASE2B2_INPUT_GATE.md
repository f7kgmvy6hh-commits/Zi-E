# Phase 2B2 Classical CAN Input Gate

Status: `WAITING_FOR_VERIFIED_INPUTS`. This checklist condenses the authoritative
analysis in `PHASE2B2_BUS_TIMING_ANALYSIS.md`; it freezes no implementation value.

| Required input | Evidence required | Status |
|---|---|---|
| Exact transceiver on ESP32 and STM32 sides | part/revision/datasheet, propagation/standby/fault behavior, received labels | WAITING |
| Physical topology/node count | reviewed schematic and as-built diagram | WAITING |
| Total cable/stub lengths and connector count | measured harness worksheet/photos | WAITING |
| Termination placement/value and cable impedance class | schematic, parts and bench measurement | WAITING |
| Controller clock source/tolerance | exact board schematic/datasheet plus configured/measured clock over relevant corners | WAITING |
| Bitrate candidates/sample timing | derived only after clocks/transceiver/topology; analysis, not selection by convention | WAITING |
| Complete message inventory | type/direction/payload before framing, periodic rate/min inter-arrival, burst, jitter, deadline, criticality | WAITING |
| Arbitration/priority needs | safety dominance, identifier candidate analysis and acceptance filtering | WAITING |
| ISR/task/queue latency | instrumented ESP32 and STM32 measurements, capacities/high-water/overflow policy | WAITING |
| Physical safe-stop requirement/evidence | measured hazard-based requirement and observed local stop behavior; link to heartbeat/lease analysis | WAITING |
| Classical CAN 8-byte constraint | explicit accounting for each semantic message; standard/extended choice remains open | KNOWN CONSTRAINT; DESIGN WAITING |
| Fragmentation/reassembly | bounds, transaction/session identity, order/duplicate/loss/timeout/reset behavior and memory cap | WAITING |
| Integrity/authentication scope | threat model; end-to-end integrity decision; sender/replay trust or authentication plan | WAITING |
| Retries/error handling | arbitration/error budget, automatic/single-shot policy, error-passive/bus-off/rejoin and no-auto-motion rules | WAITING |
| Physical signal quality | scope captures for edges/ringing/common-mode/noise and injected faults | WAITING |

## Freeze decision checklist

- [ ] Every required row has reviewed evidence and provenance.
- [ ] Priority-aware utilization/response-time analysis passes with declared margin.
- [ ] Queue/ISR latency and error/retry budgets are included, not assumed zero.
- [ ] Fragmentation cannot partially dispatch a semantic command or cross sessions.
- [ ] Integrity scope does not confuse CAN CRC with sender authentication.
- [ ] Bench loss/error/bus-off/overflow tests preserve STM32 local safe state.
- [ ] Physical safe-stop evidence supports the selected liveness/deadline contract.

Until all boxes pass: no bitrate, CAN identifier, framing, fragmentation, timeout,
retry, integrity/authentication or measured deadline may be frozen; no production CAN
implementation may start.

