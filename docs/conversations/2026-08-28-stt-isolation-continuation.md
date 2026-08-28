# Zi-E project continuation — STT isolation

## User request
Continue the previous work from the attached recovery prompt without rebuilding completed subsystems.

## Evidence reviewed
- `docs/CURRENT_STATE.md` identifies the active unfinished work as isolation of native `faster-whisper` execution.
- Existing working tree changes moved the server endpoint to `LocalTranscriber.transcribe_isolated()` and added `app.voice.stt_worker`.
- The repository's normal pytest cache path is permission-blocked on this Windows workspace; tests were run with a temporary cache directory.

## Work completed
- Preserved the subprocess isolation design.
- Hardened worker result handling against invalid UTF-8/JSON and non-dictionary output.
- Added explicit handling for worker launch failures (`STT_WORKER_UNAVAILABLE`).
- Kept empty audio fail-fast behavior, avoiding model loading.

## Verification
- `PYTHONPATH=. uv run pytest -o cache_dir="$LOCALAPPDATA/Temp/zie-pytest-cache" -q`
- Result: `34 passed in 2.73s` after adding worker launch/output regression coverage.
- `python -m compileall -q app`: passed.
- `git diff --check`: passed; Git reported only line-ending conversion notices.

## Remaining state
The STT isolation changes remain uncommitted with the pre-existing working-tree modifications. No service was restarted, no credentials were read, and no physical hardware path was changed.
