from __future__ import annotations

import asyncio
from contextlib import asynccontextmanager
from pathlib import Path

from fastapi import Depends, FastAPI, Header, HTTPException, Request, WebSocket, WebSocketDisconnect, status
from fastapi.responses import FileResponse, StreamingResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel, Field

from app.robot.state_machine import RobotController, RobotState
from app.runtime.adapter import (
    HostRuntimeAdapter,
    RequestPhase,
    SemanticCommandRequest,
    SimulatorRuntimeAdapter,
    TargetMode,
    UnavailableRuntimeAdapter,
)
from app.providers.policy import app_provider_status
from app.integrations.hermes import HermesBridge, route_message
from app.security.audit import AuditLog
from app.security.auth import BearerAuth
from app.server.config import Settings, load_settings
from app.server.control_center import (
    cockpit_status,
    developer_actions,
    inventory_preview,
    inventory_schema,
    repository_status,
    run_developer_action,
)
from app.server.events import EventBus
from app.server.metrics import system_metrics
from app.server.state import StateCore
from app.voice.providers import command_synthesizer, elevenlabs_synthesizer
from app.voice.service import VoiceService
from app.voice.stt import LocalTranscriber


class RobotCommandRequest(BaseModel):
    target: RobotState
    timeout: float = Field(gt=0, le=30)


class MuteRequest(BaseModel):
    muted: bool


class ChatRequest(BaseModel):
    message: str = Field(min_length=1, max_length=16000)


class SpeakRequest(BaseModel):
    text: str = Field(min_length=1, max_length=10000)


class DeveloperActionRequest(BaseModel):
    action_id: str = Field(min_length=1, max_length=40, pattern=r"^[a-z_]+$")


def create_app(
    settings: Settings | None = None,
    runtime_adapter: HostRuntimeAdapter | None = None,
) -> FastAPI:
    cfg = settings or load_settings()
    auth = BearerAuth(cfg.auth_token)
    events = EventBus()
    robot = RobotController(simulator=cfg.simulator)
    adapter = runtime_adapter or (
        SimulatorRuntimeAdapter(robot) if cfg.simulator else UnavailableRuntimeAdapter(robot)
    )
    initial_runtime = adapter.status()
    if cfg.simulator and initial_runtime.target_mode is not TargetMode.SIMULATION:
        raise ValueError("simulator configuration requires the simulation adapter")
    if not cfg.simulator and initial_runtime.target_mode is TargetMode.SIMULATION:
        raise ValueError("non-simulator configuration cannot use simulation authority")
    state = StateCore(cfg.simulator)
    state.update("robot", {
        "state": initial_runtime.robot_state,
        "target_mode": initial_runtime.target_mode.value,
        "authority": "available" if initial_runtime.authority_generation > 0 else "unavailable",
        "execution": initial_runtime.execution,
        "physically_confirmed": initial_runtime.physically_confirmed,
    })
    audit = AuditLog(cfg.log_path)
    voice = VoiceService(
        cfg.voice_id,
        elevenlabs_synthesizer(cfg.elevenlabs_api_key) if cfg.elevenlabs_api_key else None,
        command_synthesizer(cfg.voice_fallback_command) if cfg.voice_fallback_command else None,
        cfg.elevenlabs_model,
    )
    transcriber = LocalTranscriber()

    def runtime_status():
        return adapter.status()

    def real_runtime_ready(current) -> bool:
        return (
            current.target_mode is TargetMode.FUTURE_REAL_TARGET
            and current.readiness == "ready"
            and current.availability == "available"
            and current.authority_generation > 0
            and bool(current.session_id)
        )

    def response_is_consistent(result, before, after, *, allow_requested: bool = False) -> bool:
        if (
            result.authority_generation != after.authority_generation
            or result.target_mode is not after.target_mode
            or before.authority_generation != after.authority_generation
            or before.session_id != after.session_id
        ):
            return False
        valid_outcome = {
            RequestPhase.REQUESTED: allow_requested and not result.delivered and not result.physically_confirmed,
            RequestPhase.ACCEPTED: result.delivered and not result.physically_confirmed,
            RequestPhase.REJECTED: not result.delivered and not result.physically_confirmed,
            RequestPhase.CONFIRMED: (
                result.target_mode is TargetMode.FUTURE_REAL_TARGET
                and result.delivered
                and result.physically_confirmed
            ),
        }[result.phase]
        if not valid_outcome:
            return False
        if result.target_mode is TargetMode.REAL_TARGET_UNAVAILABLE:
            return not result.delivered and not result.physically_confirmed
        if result.target_mode is TargetMode.SIMULATION:
            return not result.physically_confirmed
        return True

    def response_execution(result) -> str:
        if result.physically_confirmed:
            return "confirmed"
        if result.target_mode is TargetMode.SIMULATION and result.delivered:
            return "simulated"
        if result.delivered:
            return "delivered_unconfirmed"
        return "not_delivered"

    async def deadman_loop():
        while True:
            await asyncio.sleep(0.05)
            before = runtime_status()
            result = adapter.poll()
            if result:
                current = runtime_status()
                if (
                    not response_is_consistent(result, before, current)
                    or (
                        current.target_mode is TargetMode.FUTURE_REAL_TARGET
                        and not real_runtime_ready(current)
                    )
                ):
                    state.update("robot", {
                        "state": "DISCONNECTED",
                        "authority": "unavailable",
                        "execution": "not_delivered",
                        "physically_confirmed": False,
                    })
                    await events.publish("runtime.feedback_rejected", {
                        "reason": "stale_or_inconsistent_authority",
                        "source": current.target_mode.value,
                    })
                    continue
                state.update("robot", {
                    "state": current.robot_state, "last_command": result.public(),
                    "execution": response_execution(result),
                    "physically_confirmed": result.physically_confirmed,
                })
                await events.publish("robot.deadman", result.public())
                audit.write("robot.deadman", command=result.public())

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
    app.state.runtime_adapter = adapter
    app.mount("/hud", StaticFiles(directory=Path(__file__).parents[1] / "hud"), name="hud-assets")

    def require_auth(authorization: str | None = Header(default=None)):
        if not auth.valid(authorization):
            raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="invalid bearer token")

    @app.get("/", include_in_schema=False)
    def hud():
        return FileResponse(Path(__file__).parents[1] / "hud" / "index.html")

    @app.get("/api/health", dependencies=[Depends(require_auth)])
    def health():
        current = runtime_status()
        providers = app_provider_status(cfg, transcriber.status())
        robot_ready = (
            current.target_mode is TargetMode.SIMULATION
            or real_runtime_ready(current)
        )
        return {
            "status": "ok" if robot_ready else "blocked",
            "readiness": {
                "app": "ready",
                "required_robot": "ready" if robot_ready else "unavailable",
                "overall": "ready" if robot_ready else "blocked",
            },
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
            "robot": current.public(),
            "providers": providers,
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
            "provider_priority": app_provider_status(cfg, transcriber.status()),
        }

    @app.get("/api/runtime", dependencies=[Depends(require_auth)])
    def public_runtime():
        return runtime_status().public()

    @app.get("/api/providers", dependencies=[Depends(require_auth)])
    def providers():
        return {"modalities": app_provider_status(cfg, transcriber.status())}

    @app.get("/api/voice/status", dependencies=[Depends(require_auth)])
    def voice_status():
        providers = app_provider_status(cfg, transcriber.status())
        return {
            "stt": {"state": transcriber.status(), "chain": providers["stt"]},
            "tts": {
                "state": "configured" if cfg.elevenlabs_api_key or cfg.voice_fallback_command else "unavailable",
                "muted": state.snapshot()["voice"]["muted"],
                "speaking": state.snapshot()["voice"]["speaking"],
                "chain": providers["tts"],
            },
            "wake": {"state": "unavailable", "chain": providers["wake"]},
        }

    @app.get("/api/hardware-profile", dependencies=[Depends(require_auth)])
    def hardware_profile():
        current = runtime_status()
        return {
            "status": "unavailable" if current.target_mode is TargetMode.REAL_TARGET_UNAVAILABLE else "not_exposed",
            "profile_id": None,
            "resolution_generation": None,
            "device_rebinding": "explicit-only",
        }

    @app.get("/api/configuration", dependencies=[Depends(require_auth)])
    def configuration():
        return {"status": "not_exposed", "generation": None, "mutable": False}

    @app.get("/api/presentation", dependencies=[Depends(require_auth)])
    def presentation():
        return {"status": "not_exposed", "context": None, "face_pack": None, "sound_pack": None}

    @app.get("/api/diagnostics", dependencies=[Depends(require_auth)])
    def diagnostics():
        current = runtime_status()
        return {
            "runtime": current.public(),
            "event_history_count": len(events.history()),
            "persistence": "not_implemented",
            "physical_commissioning": "required",
        }

    @app.get("/api/control-center", dependencies=[Depends(require_auth)])
    def control_center():
        current = runtime_status()
        provider_state = app_provider_status(cfg, transcriber.status())
        voice_state = "configured" if cfg.elevenlabs_api_key or cfg.voice_fallback_command else "unavailable"
        return cockpit_status(current.public(), provider_state, transcriber.status(), voice_state)

    @app.get("/api/developer", dependencies=[Depends(require_auth)])
    def developer_workspace():
        return {
            "repository": repository_status(),
            "actions": [action.public() for action in developer_actions().values()],
            "arbitrary_commands": False,
            "filesystem_api": False,
            "robot_authority": "NONE",
            "test_history": "current-session-events-only",
        }

    @app.post("/api/developer/action", dependencies=[Depends(require_auth)])
    async def developer_action(request: DeveloperActionRequest):
        try:
            result = await run_developer_action(request.action_id)
        except KeyError as exc:
            raise HTTPException(status_code=404, detail=str(exc)) from exc
        except RuntimeError as exc:
            raise HTTPException(status_code=409, detail=str(exc)) from exc
        await events.publish("developer.action", {
            "action_id": request.action_id,
            "state": result["state"],
            "exit_code": result["exit_code"],
            "robot_authority": "NONE",
        })
        audit.write("developer.action", action_id=request.action_id,
                    state=result["state"], exit_code=result["exit_code"])
        return result

    @app.get("/api/hardware/inventory/schema", dependencies=[Depends(require_auth)])
    def hardware_inventory_schema():
        return {
            "columns": inventory_schema(), "physical_verification": "NOT_VERIFIED",
            "authority": "non-authoritative-intake", "persistence": "not_implemented",
        }

    @app.post("/api/hardware/inventory/preview", dependencies=[Depends(require_auth)])
    async def hardware_inventory_preview(request: Request):
        try:
            result = inventory_preview(await request.body())
        except ValueError as exc:
            raise HTTPException(status_code=422, detail=str(exc)) from exc
        await events.publish("hardware.inventory.preview", {
            "count": result["count"], "physical_verification": "NOT_VERIFIED",
            "persisted": False,
        })
        audit.write("hardware.inventory.preview", count=result["count"], persisted=False)
        return result

    @app.get("/api/semantic-contracts", dependencies=[Depends(require_auth)])
    def semantic_contracts():
        return {
            "robot_state": {
                "endpoint": "/api/robot/command", "transport": "HostRuntimeAdapter",
                "states": [state.value for state in RobotState],
                "outcomes": [phase.value for phase in RequestPhase],
                "physical_confirmation": "authoritative-runtime-feedback-only",
            },
            "motion": {
                "directions": ["forward", "backward", "left", "right", "stop"],
                "speed_limit": "bounded-future-runtime-policy",
                "deadman": "required-for-motion",
                "state": "NOT_CONFIGURED",
                "raw_motor_control": False,
            },
            "actuator_commissioning": {
                "single_restrained_actuator": True, "explicit_enable": True,
                "bounded_jog": True, "verified_range_recording": "future-reviewed-evidence",
                "state": "NOT_CONFIGURED", "raw_actuator_control": False,
            },
            "presentation": {
                "face_expressions": ["neutral", "happy", "curious", "sleepy", "warning"],
                "rgb_fail_state": "fail-dark", "state": "NOT_CONFIGURED",
                "arbitrary_display_memory": False,
            },
            "firmware": {"approved_fixed_workflows_only": True, "unrestricted_flash": False},
        }

    @app.get("/api/plugins", dependencies=[Depends(require_auth)])
    def plugins():
        current = runtime_status()
        command_ready = (
            current.target_mode is TargetMode.SIMULATION
            or real_runtime_ready(current)
        )
        return {
            "plugins": [
                {"id": "chat", "status": "available" if cfg.hermes_command else "unavailable", "permission": "invoke" if cfg.hermes_command else "none"},
                {"id": "task", "status": "presentation-only", "permission": "none"},
                {
                    "id": "robot",
                    "status": (
                        current.target_mode.value if command_ready
                        else "real-target-unavailable"
                    ),
                    "permission": "semantic-command" if command_ready else "emergency-request-only",
                },
                {"id": "voice", "status": "available" if cfg.elevenlabs_api_key or cfg.voice_fallback_command else "unavailable", "permission": "speak"},
                {"id": "system", "status": "available", "permission": "read"},
                {"id": "browser", "status": "presentation-only", "permission": "none"},
                {"id": "memory", "status": "not_implemented", "permission": "none"},
            ]
        }

    @app.get("/api/extensions", dependencies=[Depends(require_auth)])
    def extensions():
        return plugins()

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
        result = await transcriber.transcribe_isolated(audio, ".wav")
        if not result["success"]:
            raise HTTPException(status_code=422, detail=result["error"])
        await events.publish("voice.transcript", {
            "provider": result["provider"], "length": len(result["transcript"]),
        })
        return result

    @app.post("/api/robot/command", dependencies=[Depends(require_auth)])
    async def robot_command(request: RobotCommandRequest):
        before = runtime_status()
        if before.target_mode is TargetMode.FUTURE_REAL_TARGET and not real_runtime_ready(before):
            raise HTTPException(status_code=503, detail="real-target runtime is not ready")
        result = adapter.submit(SemanticCommandRequest(request.target.value, request.timeout))
        after = runtime_status()
        if (
            result.target != request.target.value
            or not response_is_consistent(result, before, after)
            or (after.target_mode is TargetMode.FUTURE_REAL_TARGET and not real_runtime_ready(after))
        ):
            raise HTTPException(status_code=503, detail="runtime authority changed during command")
        state.update("robot", {
            "state": after.robot_state,
            "last_command": result.public(),
            "execution": response_execution(result),
            "physically_confirmed": result.physically_confirmed,
        })
        await events.publish("robot.command", result.public())
        await events.publish("robot.state", {
            "state": after.robot_state,
            "source": after.target_mode.value,
            "execution": response_execution(result),
            "physically_confirmed": result.physically_confirmed,
        })
        if after.target_mode is TargetMode.SIMULATION:
            await events.publish("robot.telemetry", {"mode": "SIMULATION", "state": after.robot_state})
        audit.write("robot.command", command=result.public())
        if result.phase in {RequestPhase.REQUESTED, RequestPhase.REJECTED}:
            code = 503 if after.target_mode is TargetMode.REAL_TARGET_UNAVAILABLE else 409
            raise HTTPException(status_code=code, detail=result.public())
        return result.public()

    @app.post("/api/robot/estop", dependencies=[Depends(require_auth)])
    async def estop():
        before = runtime_status()
        result = adapter.request_estop("hud")
        current = runtime_status()
        if (
            result.target != RobotState.EMERGENCY_STOP.value
            or not response_is_consistent(result, before, current, allow_requested=True)
        ):
            raise HTTPException(status_code=503, detail="runtime authority changed during E-stop request")
        state.update("robot", {
            "state": current.robot_state,
            "last_command": result.public(),
            "execution": response_execution(result),
            "physically_confirmed": result.physically_confirmed,
        })
        await events.publish("robot.estop", result.public())
        await events.publish("robot.state", {
            "state": current.robot_state, "source": current.target_mode.value,
            "execution": response_execution(result),
            "physically_confirmed": result.physically_confirmed,
        })
        audit.write("robot.estop", command=result.public())
        if result.phase in {RequestPhase.REQUESTED, RequestPhase.REJECTED}:
            raise HTTPException(status_code=503, detail=result.public())
        return result.public()

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
