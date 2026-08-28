from __future__ import annotations

import json
import sys
from pathlib import Path

from app.voice.stt import LocalTranscriber


def main() -> int:
    if len(sys.argv) != 3:
        print(json.dumps({"success": False, "error": "invalid worker arguments"}))
        return 2
    audio_path = Path(sys.argv[1])
    model = sys.argv[2]
    try:
        result = LocalTranscriber(model)._transcribe_bytes_in_process(audio_path.read_bytes(), audio_path.suffix)
        print(json.dumps(result), flush=True)
        # A typed transcription rejection is a valid protocol response. Nonzero is
        # reserved for failure of the isolation worker itself.
        return 0
    except BaseException as exc:
        print(json.dumps({"success": False, "provider": "local", "error": f"STT_WORKER_CRASHED: {type(exc).__name__}"}), flush=True)
        return 3


if __name__ == "__main__":
    raise SystemExit(main())
