from __future__ import annotations

import asyncio
from collections import deque
from datetime import datetime, timezone
from typing import Any


class EventBus:
    def __init__(self, history_size: int = 100):
        self._events: deque[dict[str, Any]] = deque(maxlen=history_size)
        self._subscribers: set[asyncio.Queue] = set()
        self._next_id = 1

    async def publish(self, event_type: str, payload: dict[str, Any]) -> dict[str, Any]:
        event = {"id": self._next_id, "type": event_type, "timestamp": datetime.now(timezone.utc).isoformat(), "payload": payload}
        self._next_id += 1
        self._events.append(event)
        for queue in tuple(self._subscribers):
            if queue.full():
                try:
                    queue.get_nowait()
                except asyncio.QueueEmpty:
                    pass
            queue.put_nowait(event)
        return event

    def history(self) -> list[dict[str, Any]]:
        return list(self._events)

    def subscribe(self, size: int = 32) -> asyncio.Queue:
        queue: asyncio.Queue = asyncio.Queue(maxsize=size)
        self._subscribers.add(queue)
        return queue

    def unsubscribe(self, queue: asyncio.Queue) -> None:
        self._subscribers.discard(queue)
