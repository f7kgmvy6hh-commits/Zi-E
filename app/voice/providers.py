from __future__ import annotations

import json
import shlex
import subprocess
from threading import Event
from urllib import request
from urllib.parse import quote


def elevenlabs_synthesizer(api_key: str):
    def synthesize(text: str, voice_id: str, model_id: str, cancelled: Event):
        endpoint = f"https://api.elevenlabs.io/v1/text-to-speech/{quote(voice_id, safe='')}"
        payload = json.dumps({"text": text, "model_id": model_id}).encode("utf-8")
        req = request.Request(endpoint, payload, {
            "xi-api-key": api_key,
            "content-type": "application/json",
            "accept": "audio/mpeg",
        })
        with request.urlopen(req, timeout=30) as response:
            while not cancelled.is_set():
                chunk = response.read(64 * 1024)
                if not chunk:
                    break
                yield chunk
    return synthesize


def command_synthesizer(command: str):
    argv = shlex.split(command, posix=False)
    if not argv:
        raise ValueError("voice fallback command is empty")

    def synthesize(text: str, voice_id: str, model_id: str, cancelled: Event):
        completed = subprocess.run([*argv, "--voice-id", voice_id], input=text.encode("utf-8"), capture_output=True, timeout=30, shell=False, check=True)
        if completed.stdout and not cancelled.is_set():
            yield completed.stdout
    return synthesize
