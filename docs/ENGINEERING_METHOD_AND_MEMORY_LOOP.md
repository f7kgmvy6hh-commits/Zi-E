# Engineering Method, Memory Loop, and AI Working Rules

## Core engineering loop
For every meaningful design change:
1. Restate requirement and measurable success criteria.
2. Research manufacturer datasheets, application notes, standards/guidance, and relevant engineering literature.
3. Generate at least one credible alternative.
4. Perform FMEA and identify single-point/common-cause failures.
5. Check tolerances, manufacturability, assembly and service access.
6. Check misuse, jams, power loss, unplugging, blocked sensors, and human-contact cases.
7. Define a test that can falsify the design assumption.
8. Re-check interfaces with every affected subsystem.
9. Run change-impact analysis on mass, COM, power, space, thermals, wiring, firmware and safety.
10. Check wear, replacement, supply-chain and future upgrade path.
11. Red-team the result and either approve conditionally, freeze, or redesign.

## Memory / inventory loop
Before answering a new project question or approving a change, review:
- current master spec;
- component matrix;
- current CAD parameters and validation;
- design decisions and decision log;
- unresolved freeze gates;
- electronics/power/safety interfaces;
- mechanical swept volumes/cables/service zones;
- AI/software boundaries;
- previous corrections and deprecated assumptions.

Ask: What did we already decide? What changed later? What has not been detailed enough? What dependency can the new change break? Which old subsystem has disappeared from the discussion but still matters?

## Conditional-decision rule
Never write only `CONDITIONAL`. Maintain:
- Candidate A (default),
- Candidate B/C (fallbacks),
- explicit switch condition,
- evidence/test needed to freeze the decision.

## AI/CAD rule
AI-generated visuals are not CAD authority. CAD authority comes from parametric source + real component envelopes + constraints + testable dimensions + validation scripts. Decorative image generation may guide industrial design only after it is checked against engineering geometry.

## Model collaboration rule
Use one model/agent as owner for a task, request an independent review only for high-risk/critical work, and avoid wasting credits by having multiple models duplicate routine work. Approved results must be promoted into this repository rather than living only in chat.


## Open-Source Harvest Loop
For each material subsystem decision: define the problem; search mature open-source robots and their issue trackers/troubleshooting docs; extract actual failure modes; adapt only the useful principle to ZI-E; check license before importing any code/CAD; add a verification test; then KEEP/REJECT. Current findings live in `OPEN_SOURCE_HARVEST_AUDIT.md`.
