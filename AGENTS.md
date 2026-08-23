# AGENTS.md — ZI-E AI Collaboration Rules

## Source of truth

This repository is the source of truth.

When a chat, legacy PDF, generated code, or AI output conflicts with an approved repository decision, stop and resolve the conflict before implementation.

## Authority

### Product Owner
The user decides:
- desired behavior;
- visual personality;
- acceptable compromises;
- budget and purchase approval;
- final product direction.

### ChatGPT — Coordinating Architect / Reviewer
Responsibilities:
- maintain project structure and current state;
- split work into small verifiable tasks;
- choose which AI should handle each task;
- write the task prompt;
- review outputs for integration conflicts;
- identify assumptions and unresolved risks;
- minimize duplicate AI effort and wasted credits;
- prevent legacy errors from silently entering the new design.

### Coding / specialist agents
Codex, Claude, Gemini, or future agents may implement or independently review scoped tasks.

No agent owns the whole project by default.

## Work rules

1. One subsystem, one owner at a time.
2. Critical work receives an independent review.
3. Do not ask three models to solve the same easy task.
4. Use additional models when the decision is expensive, safety-critical, or likely to create architecture lock-in.
5. Every implementation task must state:
   - inputs;
   - expected output;
   - files allowed to change;
   - constraints;
   - validation/test criteria.
6. Do not modify unrelated files.
7. Do not treat a generated result as approved until reviewed.
8. Hardware-control code must expose safe high-level commands, not arbitrary raw actuator control to the AI layer.
9. Secrets must never be committed.

## Legacy rule

All files under `legacy/` are untrusted reference inputs.

They may contain:
- contradictory pin mappings;
- incorrect power assumptions;
- incomplete placeholders;
- outdated API assumptions;
- unverified mechanical dimensions.

Verify from primary datasheets and the new design before reuse.

## Credit-efficiency rule

Use the cheapest/lowest-effort suitable model for routine transformations and simple implementation.

Reserve stronger reasoning / independent cross-review for:
- power architecture;
- battery safety;
- motion safety;
- mechanical interference;
- protocol architecture;
- major firmware architecture;
- final purchase BOM;
- major design freeze decisions.

## Conversation archive

After every meaningful ZI-E project session:
- archive the user-visible project discussion under `docs/conversations/`;
- update `docs/CURRENT_STATE.md`;
- promote approved decisions to `docs/DESIGN_DECISIONS.md`;
- do not rely on chat-only memory for important project state.

## Open-source harvest rule

Before freezing an important mechanical, electrical, controls, or AI subsystem, inspect mature open-source robotics projects **including their issue trackers and troubleshooting docs**, not only polished README pages.

Required flow: `problem -> comparable projects -> real failure reports -> adapted ZI-E mitigation -> simulation/bench verification -> decision`.

Record material findings in `docs/OPEN_SOURCE_HARVEST_AUDIT.md`. Do not copy code/CAD into ZI-E until license compatibility and provenance are checked. Engineering ideas may inform the design, but imported artifacts need explicit license/source review.

## Safe commissioning rule

No newly assembled robot may enter normal autonomous motion before a low-power commissioning pass verifies actuator identity, model/voltage class, bus/baud, direction, zero/range, end-state sensors, and plausible feedback. A received command is not considered executed until actual state confirms it.
