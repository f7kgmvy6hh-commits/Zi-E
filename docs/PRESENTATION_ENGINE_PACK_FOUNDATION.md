# Presentation Engine and Face/Sound Pack Foundation

## Scope and authority

This C++17 host-only layer consumes opaque commands already accepted by the Semantic
Robot API. It handles semantic expression, speech, and sound-cue intents and writes
state only through `AuthoritativeRobotCore`. It has no framebuffer, GPIO, PWM, audio
device, codec, filesystem loader, network, persistence, UI editor, dynamic loading, or
hardware ownership.

## Data-only packs

Face and sound packs bind stable pack/version/schema data to an authoritative extension
registry package, logical device, and hardware profile. Their registry manifests must
be `asset_pack` records in the matching `face_pack` or `sound_pack` category, with no
entrypoint and no permissions. Pack data contains no trust, controller ownership,
permissions, or executable callback.

Face packs declare named expressions, abstract `asset.*` handles, optional semantic
parameters, a required default and fallback expression, and semantic transitions whose
endpoints must exist. Sound packs declare named speech or non-speech cues and abstract
`asset.*` handles. Schema/domain enum values, duplicate names, unknown references,
missing defaults/fallbacks, and non-abstract asset handles fail closed.

## Selection and execution

The bounded in-memory catalog keeps pack declaration/validation state separate from
contextual selection. A validated pack may be selected by zero, one, or many contexts;
each context has at most one selected face pack and one selected sound pack. Replacing
one context does not affect another. Activation requires the pack's exact active
registry capability and replacement is deterministic. Read lookup never creates a
context. Every selected-pack lookup rechecks registry lifecycle and capability state;
detected failure, quarantine, disablement, removal, or capability loss purges that pack
from every affected context. Registry recovery does not restore selections implicitly:
each desired context requires explicit activation.

The presentation engine seeds a new pack transition from its declared default
expression, resolves unknown expression names to its declared fallback, selects named
sound cues exactly, and advances authoritative presentation generation only after the
core state write succeeds. Rejected or unaccepted commands have no presentation side
effect. `VirtualRobot` can delegate accepted presentation/audio tokens to this engine
for deterministic host simulation; no real rendering or playback occurs.
