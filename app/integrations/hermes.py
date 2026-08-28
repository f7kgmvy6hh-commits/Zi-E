from __future__ import annotations

import asyncio
from collections.abc import AsyncIterator, Callable
from dataclasses import dataclass


@dataclass(frozen=True)
class HermesRoute:
    provider: str
    model: str
    reason: str

    def public(self) -> dict[str, str]:
        return {"provider": self.provider, "model": self.model, "reason": self.reason}


def route_message(message: str, main_model: str) -> HermesRoute | None:
    """Select the configured LLM primary without inventing fallback authority.

    Hermes slash commands own explicit model selection, so they are passed through
    without provider/model flags. Bounded fallback belongs to the authoritative
    modality chain, not prompt heuristics in this App adapter.
    """
    if message.lstrip().lower().startswith("/model"):
        return None
    provider, separator, model = main_model.partition("/")
    if not separator or not provider or not model:
        raise ValueError("main Hermes model must be provider/model")
    return HermesRoute(provider, model, "configured-primary")


class HermesBridge:
    """Thin persistent-session adapter; provider credentials never cross this API."""

    def __init__(self, session_id: str, transport: Callable):
        self.session_id = session_id
        self._transport = transport
        self.last_route: HermesRoute | None = None

    @classmethod
    def executable(
        cls,
        command: str,
        session_id: str,
        main_model: str,
        *,
        spawn=asyncio.create_subprocess_exec,
    ) -> "HermesBridge":
        bridge: HermesBridge

        async def transport(session: str, message: str):
            route = route_message(message, main_model)
            bridge.last_route = route
            argv = [
                command, "chat", "--continue", session, "--create-if-missing",
                "-q", message, "-Q", "--source", "tool",
            ]
            if route:
                argv.extend(("--provider", route.provider, "--model", route.model))
            process = await spawn(
                *argv,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE,
            )
            stdout, stderr = await process.communicate()
            if process.returncode:
                detail = stderr.decode("utf-8", errors="replace").strip()
                raise RuntimeError(f"Hermes exited with {process.returncode}: {detail[:500]}")
            if stdout:
                yield stdout

        bridge = cls(session_id, transport)
        return bridge

    async def stream(self, message: str) -> AsyncIterator[str]:
        if not message.strip():
            return
        async for chunk in self._transport(self.session_id, message):
            yield chunk.decode("utf-8") if isinstance(chunk, bytes) else chunk

    async def stream_bytes(self, message: str) -> AsyncIterator[bytes]:
        if not message.strip():
            return
        async for chunk in self._transport(self.session_id, message):
            yield chunk if isinstance(chunk, bytes) else chunk.encode("utf-8")
