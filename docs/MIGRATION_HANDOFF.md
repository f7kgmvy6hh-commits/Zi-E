# Migration Handoff — Moving ZI-E to Another ChatGPT Account

This file exists so the project can continue in a different AI account without rebuilding context from memory.

## Instructions for the new ChatGPT session

Upload or connect this repository and say:

> This repository is the source of truth for my ZI-E robot project. Read `AGENTS.md`, `PROJECT_CONTEXT.md`, `docs/DESIGN_DECISIONS.md`, `docs/ARCHITECTURE.md`, and `docs/CURRENT_STATE.md` before proposing changes. Treat everything under `legacy/` as untrusted reference only. Continue from the next task in `docs/CURRENT_STATE.md`. Do not silently overwrite established design decisions; flag conflicts first.

## Required reading order

1. `AGENTS.md`
2. `PROJECT_CONTEXT.md`
3. `docs/DESIGN_DECISIONS.md`
4. `docs/ARCHITECTURE.md`
5. `docs/CURRENT_STATE.md`
6. `docs/AI_WORKFLOW.md`
7. `docs/CHAT_DECISION_LOG.md`
8. Legacy files only when a historical detail is needed.

## What must survive migration

- ZI-E is a desktop AI companion.
- The new design is not governed by the old WALL-E guides.
- Initial AI brain is on the laptop via wireless connection.
- A used phone is a future edge-brain integration.
- Protected Shutdown is a physical compact/protective state, not just software power-off.
- Hardware should be modular and serviceable.
- The current phase is **Design Freeze v0.1**, not hardware purchase or final coding.

## Handoff discipline

Before ending a significant future session:
1. Update `docs/CURRENT_STATE.md`.
2. Add new approved decisions to `docs/DESIGN_DECISIONS.md`.
3. Update architecture if it changed.
4. Commit the changes.
5. Avoid relying on chat-only memory.

## Conversation archive

Historical project discussion is under `docs/conversations/`.

A new AI account should read the concise source-of-truth files first, then use the conversation archive when it needs rationale or historical context.

Future sessions should follow `docs/CONVERSATION_ARCHIVE_POLICY.md`.
