# Software Architecture Freeze Candidate — 2026-08-28

Status: `READY`, subject to repository review and commit. This freezes the host
software foundation, not physical hardware behavior. Phase 2B2 remains
`WAITING_FOR_VERIFIED_INPUTS`.

## Authority chain

`PackagePolicy` decides whether declared content is verified, compatible, and
eligible to activate. It never grants trust, capability, controller, profile, or
device authority. `ExtensionRegistry` owns those facts and their authorization
generation. `HardwareProfileManager` resolves only exact live registry facts and
never substitutes a device. `ExtensionHost` alone issues executable plugin contexts,
epochs, sessions, and capability-scoped services. `HostRuntime` coordinates these
authorities in dependency order; it cannot override any denial. The internal
`AuthoritativeRobotCore` accepts only already-authorized semantic tokens and is not an
SDK surface.

The only host/plugin motion-intent route is the Semantic Robot API. Plugin, provider,
presentation, package, profile, and configuration contracts contain no raw actuator,
GPIO, PWM, register, driver, CAN-frame, setpoint, or protected-safety escape hatch.

Every shutdown, required failure, quarantine, disable, removal, update, rollback, and
recovery retires dependent authority. Reactivation requires fresh registry generation,
SDK epoch, command session, and profile resolution generation as applicable. A
checkpoint is bounded copied intent, never authority. Unknown enum, lifecycle,
contract, identity, and domain values fail closed.

## Frozen now

- Subsystem responsibility boundaries and the authority ownership chain above.
- Semantic command/event/read-only-state API boundaries.
- Package declaration, verification, compatibility, activation, quarantine,
  update, rollback, and fresh-authority lifecycle model.
- Distinct package, physical device, logical instance, controller, profile, content,
  authorization-generation, epoch, and session identities.
- Four independent provider modality chains: LLM, STT, TTS, and Wake. Each uses exact
  semantic capability, authoritative ordered priority, bounded typed failover, and
  live authorization/health eligibility. OpenAI-capable entries are preferred when
  configured and authorized; this is a semantic policy, not a network implementation.
- Data-only face/sound pack catalog and context-owned selection; registry revocation
  purges selection and recovery requires explicit reactivation.
- Runtime startup, readiness, required-versus-optional failure dominance, reverse
  revocation, semantic checkpoints, exact-device recovery, and explicit reactivation.
- Package/security/watchdog policy boundaries. No policy decision is a capability
  grant and no watchdog action performs raw control.
- Protected safety separation: STM32 owns local motion/safety enforcement; ESP32-S3
  owns multimedia, presence, behavior orchestration, and semantic intent submission.
- AI/plugins cannot access raw actuators. `AuthoritativeRobotCore`, drivers, and
  protected safety remain internal.
- VirtualRobot integration contract: accepted semantic token -> deterministic
  simulated execution -> authoritative state update -> immutable event. Simulation
  proves software flow only and never physical commissioning.

## Not frozen — verify later

- CAN bitrate, identifiers, fragmentation, transceiver, and physical bus topology.
- Timing deadlines based on measurements, ISR/queue latency, clock tolerance, and
  physical safe-stop timing.
- Real pins, wiring lengths, sensor performance, motor torque/current/thermal limits,
  and measured battery behavior.
- Display, camera, and audio physical interfaces that remain unverified.
- Final CAD dimensions, clearances, mass, center of gravity, and interfaces dependent
  on received parts.

No placeholder estimate may be promoted to a verified value. These items block real
drivers, CAN/wire freeze, and physical commissioning, but do not block this software
foundation freeze.

## Future App boundary

The future App may consume public services for runtime readiness, explicit profile
selection, package/plugin management, modality priority configuration, face/sound
selection, semantic commands, transactional configuration, logs/events/diagnostics,
commissioning workflow, and VirtualRobot-versus-real target selection. It must never
construct authority, call `AuthoritativeRobotCore`, bypass `HostRuntime`, mutate the
registry directly as a convenience, or expose raw actuator control.

