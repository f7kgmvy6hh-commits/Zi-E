import asyncio

from app.voice.stt import LocalTranscriber


class CompletedProcess:
    returncode = 0
    killed = False

    async def communicate(self):
        return self.stdout, b""

    def kill(self):
        self.killed = True

    async def wait(self):
        self.returncode = -1


def test_isolated_transcriber_rejects_invalid_worker_output(monkeypatch):
    process = CompletedProcess()
    process.stdout = b"not-json"

    async def fake_create_process(*args, **kwargs):
        return process

    monkeypatch.setattr(asyncio, "create_subprocess_exec", fake_create_process)

    result = asyncio.run(LocalTranscriber().transcribe_isolated(b"audio"))

    assert result["error"] == "STT_WORKER_INVALID_OUTPUT"


def test_isolated_transcriber_reports_worker_launch_failure(monkeypatch):
    async def fake_create_process(*args, **kwargs):
        raise OSError("executable missing")

    monkeypatch.setattr(asyncio, "create_subprocess_exec", fake_create_process)

    result = asyncio.run(LocalTranscriber().transcribe_isolated(b"audio"))

    assert result["error"] == "STT_WORKER_UNAVAILABLE"


def test_isolated_transcriber_accepts_strict_success(monkeypatch):
    process = CompletedProcess()
    process.stdout = b'{"success":true,"transcript":"hello","provider":"local"}'

    async def fake_create_process(*args, **kwargs):
        return process

    monkeypatch.setattr(asyncio, "create_subprocess_exec", fake_create_process)
    result = asyncio.run(LocalTranscriber().transcribe_isolated(b"audio"))

    assert result == {"success": True, "transcript": "hello", "provider": "local"}


def test_isolated_transcriber_preserves_typed_handled_failure(monkeypatch):
    process = CompletedProcess()
    process.stdout = b'{"success":false,"transcript":"","provider":"local","error":"audio rejected"}'

    async def fake_create_process(*args, **kwargs):
        return process

    monkeypatch.setattr(asyncio, "create_subprocess_exec", fake_create_process)
    result = asyncio.run(LocalTranscriber().transcribe_isolated(b"audio"))

    assert result["error"] == "audio rejected"


def test_isolated_transcriber_rejects_provider_impersonation(monkeypatch):
    process = CompletedProcess()
    process.stdout = b'{"success":true,"transcript":"hello","provider":"openai"}'

    async def fake_create_process(*args, **kwargs):
        return process

    monkeypatch.setattr(asyncio, "create_subprocess_exec", fake_create_process)
    result = asyncio.run(LocalTranscriber().transcribe_isolated(b"audio"))

    assert result["error"] == "STT_WORKER_INVALID_OUTPUT"


def test_isolated_transcriber_kills_worker_on_timeout(monkeypatch):
    process = CompletedProcess()

    async def communicate_forever():
        await asyncio.Future()

    process.communicate = communicate_forever

    async def fake_create_process(*args, **kwargs):
        return process

    async def immediate_timeout(awaitable, timeout):
        awaitable.close()
        raise asyncio.TimeoutError

    monkeypatch.setattr(asyncio, "create_subprocess_exec", fake_create_process)
    monkeypatch.setattr(asyncio, "wait_for", immediate_timeout)
    result = asyncio.run(LocalTranscriber().transcribe_isolated(b"audio"))

    assert result["error"] == "STT_WORKER_TIMEOUT"
    assert process.killed
