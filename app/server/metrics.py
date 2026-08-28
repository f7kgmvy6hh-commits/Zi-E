from __future__ import annotations

import os
import shutil

import psutil


def system_metrics() -> dict:
    memory = psutil.virtual_memory()
    disk = shutil.disk_usage(os.getcwd())
    return {
        "source": "local-machine",
        "cpu_percent": psutil.cpu_percent(interval=None),
        "memory_percent": memory.percent,
        "memory_available_bytes": memory.available,
        "disk_free_bytes": disk.free,
        "process_uptime_seconds": max(0.0, psutil.Process().create_time() and __import__("time").time() - psutil.Process().create_time()),
    }
