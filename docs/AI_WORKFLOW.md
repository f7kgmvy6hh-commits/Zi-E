# AI Workflow and Credit Strategy

## Goal

Use several AI systems productively without duplicated work, conflicting edits, or unnecessary credit consumption.

## Planned participants

- User — Product Owner
- ChatGPT — coordinating architect, reviewer, prompt writer, project-state keeper
- Codex — coding/implementation when appropriate
- Claude — architecture/code review or implementation when appropriate
- Gemini — independent review, research, alternative implementation when appropriate

The exact assignment can change per task.

## Default workflow

```text
User goal
   ↓
ChatGPT scopes task
   ↓
Select one primary agent
   ↓
Implementation / analysis
   ↓
Independent review only if warranted
   ↓
ChatGPT integration review
   ↓
Tests / acceptance criteria
   ↓
Commit + update CURRENT_STATE / decisions
```

## When to use multiple agents

Use independent cross-review when:
- wrong answer could damage hardware;
- purchase decision is expensive;
- power/battery safety is involved;
- a major architecture becomes difficult to reverse;
- firmware touches motion safety;
- external API assumptions are uncertain;
- first agent's answer contains unresolved risks.

Do not use multiple accounts/models merely to repeat the same low-risk task.

## Prompt requirements

A task prompt should include:
- goal;
- source files;
- exact files that may be changed;
- prohibited changes;
- known decisions;
- acceptance criteria;
- tests;
- expected output format;
- instruction to flag assumptions rather than invent them.

## Repository update rule

Every meaningful milestone should update at least one of:
- `docs/CURRENT_STATE.md`
- `docs/DESIGN_DECISIONS.md`
- `docs/ARCHITECTURE.md`
- implementation docs/tests

This is essential for later account migration.
