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
