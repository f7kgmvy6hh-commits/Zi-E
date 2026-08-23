# ZI-E — Desktop AI Companion Robot

ZI-E is a modular desktop companion robot project currently in **Concept Design / Pre-Engineering**.

The current design direction was defined conversationally first, then captured here so the repository—not any single AI chat—becomes the project's source of truth.

## Current status

**Concept Design v0.1 is complete enough to begin engineering freeze work.**

No hardware component, pinout, power architecture, final dimension, or production firmware should be treated as finalized yet.

### Next engineering milestone

**Design Freeze v0.1**
1. Establish real external dimensions and proportions.
2. Define internal volumes and service clearances.
3. Define head retraction envelope and arm motion envelope.
4. Define wheel/battery/mechanical zones.
5. Only then review and choose parts one-by-one.
6. Freeze electrical architecture after component validation.
7. Build simulation/prototype before purchasing the full hardware set.

## Read these first

1. `PROJECT_CONTEXT.md`
2. `docs/DESIGN_DECISIONS.md`
3. `docs/ARCHITECTURE.md`
4. `docs/CURRENT_STATE.md`
5. `AGENTS.md`
6. `docs/MIGRATION_HANDOFF.md`

## Important rule about legacy files

Everything under `legacy/` is **reference material only**. The old WALL-E-derived theory, guides, and firmware contain known contradictions and unverified assumptions.

**Do not copy legacy wiring, pinouts, power decisions, dimensions, or firmware into the new ZI-E design without re-verification.**

## Repository structure

```text
ZI-E/
├── README.md
├── PROJECT_CONTEXT.md
├── AGENTS.md
├── docs/
│   ├── DESIGN_DECISIONS.md
│   ├── ARCHITECTURE.md
│   ├── CURRENT_STATE.md
│   ├── CHAT_DECISION_LOG.md
│   ├── AI_WORKFLOW.md
│   ├── LEGACY_AUDIT.md
│   ├── ROADMAP.md
│   └── MIGRATION_HANDOFF.md
├── assets/
│   ├── branding/
│   └── concepts/
├── legacy/
│   ├── guides/
│   └── firmware/
├── mechanical/
├── electronics/
├── firmware/
├── software/
├── simulation/
└── tests/
```

## Conversation history

Project chat history is preserved under `docs/conversations/`.

See `docs/CONVERSATION_ARCHIVE_POLICY.md` for the rule that keeps future sessions portable between AI accounts.
