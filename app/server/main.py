from __future__ import annotations

import asyncio
from contextlib import asynccontextmanager
from pathlib import Path

from fastapi import Depends, FastAPI, Header, HTTPException, Request, WebSocket, WebSocketDisconnect, status
from fastapi.responses import FileResponse, StreamingResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel, Field

from app.robot.state_machine import RobotController
from app.integrations.hermes import HermesBridge, route_message
from app.security.audit import AuditLog
from app.security.auth import BearerAuth
from app.server.config import Settings, load_settings
from app.server.events import EventBus
from app.server.metrics import system_metrics
from app.server.state import StateCore
from app.voice.providers import command_synthesizer, elevenlabs_synthesizer
from app.voice.service import VoiceService
from app.voice.stt import LocalTranscriber


class RobotCommandRequest(BaseModel):
    target: str
    timeout: float = Field(gt=0, le=30)


class MuteRequest(BaseModel):
    muted: bool


class ChatRequest(BaseModel):
    message: str = Field(min_length=1, max_length=16000)


class SpeakRequest(BaseModel):
    text: str = Field(min_length=1, max_length=10000)


def create_app(settings: Settings | None = None) -> FastAPI:
    cfg = settings or load_settings()
    auth = BearerAuth(cfg.auth_token)
    events = EventBus()
    state = StateCore(cfg.simulator)
    robot = RobotController(simulator=cfg.simulator)
    audit = AuditLog(cfg.log_path)
    voice = VoiceService(
        cfg.voice_id,
        elevenlabs_synthesizer(cfg.elevenlabs_api_key) if cfg.elevenlabs_api_key else None,
        command_synthesizer(cfg.voice_fallback_command) if cfg.voice_fallback_command else None,
        cfg.elevenlabs_model,
    )
    transcriber = LocalTranscriber()

    async def deadman_loop():
        while True:
            await asyncio.sleep(0.05)
            result = robot.poll()
            if result:
                state.update("robot", {"state": robot.state.value, "last_command": result.as_dict()})
                await events.publish("robot.deadman", result.as_dict())
                audit.write("robot.deadman", command=result.as_dict())

    @asynccontextmanager
    async def lifespan(_: FastAPI):
        task = asyncio.create_task(deadman_loop())
        audit.write("server.started", host=cfg.host, port=cfg.port, simulator=cfg.simulator)
        try:
            yield
        finally:
            task.cancel()
            audit.write("server.stopped")

    app = FastAPI(title="ZI-E Command Center", version="1.0.0", lifespan=lifespan, docs_url=None, redoc_url=None)
    app.state.settings = cfg
    app.state.events = events
    app.state.core = state
    app.state.robot = robot
    app.mount("/hud", StaticFiles(directory=Path(__file__).parents[1] / "hud"), name="hud-assets")

    def require_auth(authorization: str | None = Header(default=None)):
        if not auth.valid(authorization):
            raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="invalid bearer token")

    @app.get("/", include_in_schema=False)
    def hud():
        return FileResponse(Path(__file__).parents[1] / "hud" / "index.html")

    @app.get("/api/health", dependencies=[Depends(require_auth)])
    def health():
        return {
            "status": "ok",
            "system": system_metrics(),
            "simulator": cfg.simulator,
            "hermes": {"configured": bool(cfg.hermes_command), "session": cfg.hermes_session},
            "model": cfg.hermes_main_model,
            "voice": {
                "provider": "elevenlabs" if cfg.elevenlabs_api_key else "zi-nanami-command" if cfg.voice_fallback_command else None,
                "voice_id": cfg.voice_id,
                "configured": bool(cfg.elevenlabs_api_key or cfg.voice_fallback_command),
            },
            "stt": {"provider": "local", "model": transcriber.model_name, "status": transcriber.status()},
            "robot": {"state": robot.state.value, "simulator": cfg.simulator},
        }

    @app.get("/api/state", dependencies=[Depends(require_auth)])
    def get_state():
        return state.snapshot()

    @app.get("/api/settings", dependencies=[Depends(require_auth)])
    def public_settings():
        return {
            "simulator": cfg.simulator,
            "voice_id": cfg.voice_id,
            "elevenlabs_configured": bool(cfg.elevenlabs_api_key),
            "voice_fallback_configured": bool(cfg.voice_fallback_command),
            "hermes_configured": bool(cfg.hermes_command),
            "hermes_session": cfg.hermes_session,
        }

    @app.get("/api/plugins", dependencies=[Depends(require_auth)])
    def plugins():
        return {
            "plugins": [
                {"id": "chat", "status": "available", "permission": "read-write"},
                {"id": "task", "status": "available", "permission": "read"},
                {"id": "robot", "status": "simulation" if cfg.simulator else "disconnected", "permission": "safe-command"},
                {"id": "voice", "status": "available" if cfg.elevenlabs_api_key or cfg.voice_fallback_command else "unavailable", "permission": "speak"},
                {"id": "system", "status": "available", "permission": "read"},
                {"id": "browser", "status": "available", "permission": "read"},
                {"id": "memory", "status": "available", "permission": "read"},
            ]
        }

    @app.post("/api/chat", dependencies=[Depends(require_auth)])
    async def chat(request: ChatRequest):
        if not cfg.hermes_command:
            raise HTTPException(status_code=503, detail="Hermes bridge is not configured")
        bridge = HermesBridge.executable(
            cfg.hermes_command, cfg.hermes_session, cfg.hermes_main_model
        )
        async def output():
            route = route_message(request.message, cfg.hermes_main_model)
            await events.publish(
                "chat.model_route",
                route.public() if route else {"provider": "explicit", "model": "explicit", "reason": "user-request"},
            )
            await events.publish("chat.tool", {"tool": "hermes-cli", "status": "started"})
            byte_count = 0
            async for chunk in bridge.stream_bytes(request.message):
                byte_count += len(chunk)
                yield chunk
            await events.publish("chat.response", {"bytes": byte_count, "status": "complete"})
        audit.write("hermes.request", session=cfg.hermes_session, message_length=len(request.message))
        return StreamingResponse(output(), media_type="text/plain; charset=utf-8")

    @app.post("/api/voice/mute", dependencies=[Depends(require_auth)])
    async def voice_mute(request: MuteRequest):
        result = voice.mute(request.muted)
        state.update("voice", {"muted": request.muted, "speaking": False})
        await events.publish("voice.mute", {"muted": request.muted})
        return result.__dict__

    @app.post("/api/voice/speak", dependencies=[Depends(require_auth)])
    async def voice_speak(request: SpeakRequest):
        provider = "elevenlabs" if cfg.elevenlabs_api_key else (
            "zi-nanami-command" if cfg.voice_fallback_command else None
        )
        if not provider:
            raise HTTPException(status_code=503, detail="voice synthesis is not configured")

        async def audio():
            byte_count = 0
            await events.publish("voice.speak", {
                "provider": provider, "voice_id": cfg.voice_id, "model": cfg.elevenlabs_model,
            })
            try:
                for chunk in voice.speak_chunks(request.text):
                    byte_count += len(chunk)
                    yield chunk
            finally:
                voice.stop()
                state.update("voice", {"speaking": False})
            await events.publish("voice.response", {"bytes": byte_count, "status": "complete"})

        state.update("voice", {"speaking": True})
        return StreamingResponse(audio(), media_type="audio/mpeg")

    @app.post("/api/voice/stop", dependencies=[Depends(require_auth)])
    async def voice_stop():
        result = voice.stop()
        state.update("voice", {"speaking": False})
        await events.publish("voice.stop", {"result": result.result})
        return result.__dict__

    @app.post("/api/stt/transcribe", dependencies=[Depends(require_auth)])
    async def stt_transcribe(request: Request):
        audio = await request.body()
        if len(audio) > 25 * 1024 * 1024:
            raise HTTPException(status_code=413, detail="audio payload is too large")
        result = transcriber.transcribe_bytes(audio, ".wav")
        if not result["success"]:
            raise HTTPException(status_code=422, detail=result["error"])
        await events.publish("voice.transcript", {"provider": "local", "length": len(result["transcript"])})
        return result

    @app.post("/api/robot/command", dependencies=[Depends(require_auth)])
    async def robot_command(request: RobotCommandRequest):
        result = robot.command("hud", request.target, request.timeout)
        state.update("robot", {"state": robot.state.value, "last_command": result.as_dict()})
        await events.publish("robot.command", result.as_dict())
        await events.publish("robot.state", {"state": robot.state.value, "source": "simulator" if cfg.simulator else "physical"})
        if cfg.simulator:
            await events.publish("robot.telemetry", {"mode": "SIMULATION", "state": robot.state.value})
        audit.write("robot.command", command=result.as_dict())
        if result.result.startswith("rejected"):
            raise HTTPException(status_code=409, detail=result.as_dict())
        return result.as_dict()

    @app.post("/api/robot/estop", dependencies=[Depends(require_auth)])
    async def estop():
        result = robot.emergency_stop("hud")
        state.update("robot", {"state": robot.state.value, "last_command": result.as_dict()})
        await events.publish("robot.estop", result.as_dict())
        await events.publish("robot.state", {"state": robot.state.value, "source": "simulator" if cfg.simulator else "physical"})
        audit.write("robot.estop", command=result.as_dict())
        return result.as_dict()

    @app.websocket("/api/events")
    async def websocket_events(websocket: WebSocket):
        await websocket.accept()
        try:
            credentials = await asyncio.wait_for(websocket.receive_json(), timeout=5.0)
        except (asyncio.TimeoutError, ValueError, WebSocketDisconnect):
            await websocket.close(code=4401)
            return
        if not auth.valid(credentials.get("authorization") if isinstance(credentials, dict) else None):
            await websocket.close(code=4401)
            return
        queue = events.subscribe()
        try:
            for event in events.history():
                await websocket.send_json(event)
            while True:
                await websocket.send_json(await queue.get())
        except WebSocketDisconnect:
            pass
        finally:
            events.unsubscribe(queue)

    return app


app = None


def run() -> None:
    import uvicorn
    settings = load_settings()
    uvicorn.run(create_app(settings), host=settings.host, port=settings.port, log_config=None)


if __name__ == "__main__":
    run()
