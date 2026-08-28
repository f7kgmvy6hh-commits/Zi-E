# Troubleshooting

For HTTP 401, use the `.env` bearer token without logging it. For chat 503, configure
`ZIE_HERMES_COMMAND=hermes`, confirm Hermes 0.20.6, and authenticate it separately.
Keep the session name stable.

For voice 503, configure ElevenLabs or the Zi-Nanami fallback. Stop affects remaining
chunks, not buffered audio. If startup reports an ownership file, follow recovery
instead of killing arbitrary PIDs. The pytest configuration uses writable runtime
cache paths and treats unexpected warnings as failures.
