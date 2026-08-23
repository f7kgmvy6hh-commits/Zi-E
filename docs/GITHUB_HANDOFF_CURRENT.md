# GitHub Handoff — Current Snapshot

This repository is intended to be the portable source of truth for ZI-E across ChatGPT accounts and other AI tools.

## Start here
1. `README.md`
2. `docs/ZI-E_MASTER_SPEC.md`
3. `docs/COMPONENT_MASTER_MATRIX.md`
4. `docs/CURRENT_STATE.md`
5. `docs/ENGINEERING_METHOD_AND_MEMORY_LOOP.md`
6. `mechanical/cad/current/docs/VALIDATION_REPORT.md`
7. `docs/OPEN_ITEMS_AND_FREEZE_GATES.md`

## Authority order
1. Current explicit user correction/decision
2. Current master spec/current CAD + measured tests
3. Approved design-decision log
4. Manufacturer datasheet/application notes
5. Research notes
6. Legacy files (reference only)
7. Decorative/generated concept art (visual reference only)

## GitHub upload
Upload the contents of the `ZI-E/` folder as the repository root. Large binary CAD files are currently below GitHub's single-file hard limit, but Git LFS is recommended long-term if the CAD history grows substantially.

## Branch policy
Use `main` as the current stable design branch. Use feature branches for major CAD/electronics/firmware experiments once normal Git workflow is available.
