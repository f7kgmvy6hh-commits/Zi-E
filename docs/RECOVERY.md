# Recovery

On failed health startup, the start script invokes the owned-process stop path.
`runtime/owned-processes.json` records only processes launched by ZI-E. Stop
revalidates each PID command line before termination, protecting unrelated processes
after PID reuse.

For a stale file, inspect every PID and command line. If none exists, remove only
that ownership file. Preserve audit logs. After a robot incident, emergency-stop,
remove motion power, and repeat low-power commissioning before normal operation.
