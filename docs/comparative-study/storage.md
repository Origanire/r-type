# Storage

## What the project stores today

| Data | Where | Persistence |
|---|---|---|
| Game state (entities, lives, scores) | Server memory (`ServerWorld`, `Room`) | Lost when the server stops — a round is a short arcade session |
| Client presentation state | Client memory (`ClientWorld`) | Rebuilt from snapshots |
| Game rules and tuning | `GameConstants.hpp` | Compiled in |
| Sprites | `assets/` image files, copied next to the client | Read-only files |
| Key bindings | Defaults in `KeyBindings` | Not saved yet |

Keeping live state in memory is a deliberate choice: the simulation runs 60 times per second and
needs direct access; nothing in a round has to survive a restart.

## Options for persistent data

| Technique | Persistence | Reliability | Constraints | Fits |
|---|---|---|---|---|
| Compiled constants | None (rebuild to change) | Total | Requires recompiling | Current tuning values |
| **JSON files** | Yes | Human-editable, can be invalid → must be validated | Small, read at startup | Settings, key bindings, enemy and wave definitions |
| Binary files | Yes | Compact, versioning needed | Not human-editable | Replays, large data |
| SQLite | Yes, transactional | Atomic writes, survives crashes | Embedded library | Global scoreboard, user accounts |
| Database server (PostgreSQL…) | Yes | High, with backups | Separate service to operate | Multi-server rankings |

## Planned use

- **JSON** (nlohmann-json, already in `vcpkg.json`) for client settings (key bindings, accessibility
  options) and data-driven content, so that game designers can add enemies without recompiling.
  Files are validated on load; an invalid file falls back to defaults instead of crashing.
- **SQLite** if a persistent scoreboard is added, for atomic writes on the server.
- Settings are written to a temporary file then renamed, so a crash during saving never corrupts
  the previous file.
