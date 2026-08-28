# ZI-E Hermes bridge protocol

This directory defines the thin boundary; it does not modify or duplicate Hermes.
Configure `ZIE_HERMES_COMMAND` as the Hermes executable. ZI-E invokes verified
Hermes 0.20.6 directly:

```text
hermes chat --continue zie-command-center --create-if-missing -q MESSAGE -Q --source tool
```

The stable name preserves one session. ZI-E adds provider/model flags for deterministic
routing; explicit `/model` messages omit those flags. Hermes owns tools, OAuth, and
credentials. A nonzero exit is an integration failure.

The browser sees only the ZI-E bearer token and streamed response. It never receives
Hermes credentials, OAuth tokens, provider keys, command lines, or environment data.
ZI-E neither interprets Hermes tool calls nor exposes an alternative agent loop.
