# World values, player values and rules

Three tables of the game file let scripts, the HUD and the lobby share numbers without any C++.

## World values

Declared in `worldValues`, set by scripts, sent in every snapshot, shown by the HUD.

```json
"worldValues": [
    {"name": "stage"},
    {"name": "phase", "labels": ["intro", "running", "boss", "clear", "victory", "defeat"]},
    {"name": "progress"},
    {"name": "bossHealth"},
    {"name": "shake"},
    {"name": "skip"}
]
```

| In a script | In the HUD |
|---|---|
| `world.set("stage", 2)` | `"BoundText": {"format": "STAGE {world.stage}"}` |
| `world.set_label("phase", "boss")` | `"VisibleWhen": {"key": "world.phase", "value": "boss"}` |
| `world.set("progress", 75)` | `"ProgressBar": {"key": "world.progress", "max": 100}` |
| `world.get("skip")` | (used between scripts, the console sets it) |

A value with `labels` is stored as an index and shown as its label. Values are 16-bit unsigned integers (0 to
65535). At most 32 world values.

## Player values

Declared in `playerValues`, one copy per player, set by the player's avatar script.

```json
"playerValues": [
    {"name": "weapon", "labels": ["SINGLE", "DOUBLE", "SPREAD"]},
    {"name": "force", "labels": ["", "FORCE", "FORCE OUT"]},
    {"name": "charge"}
]
```

```python
e.set_player_value("weapon", 1)                 # in powerup.py
```

```json
"BoundText": {"format": "{player0.weapon} {player0.force}"}
```

Built-in player keys, always available: `player<N>.present`, `.alive`, `.health` (0 to 1), `.score`.

## Rules

Declared in `rules`, chosen by the room creator, fixed for the whole game of that room.

```json
"rules": [
    {"name": "difficulty", "labels": ["NORMAL", "HARD", "EASY"]},
    {"name": "friendlyFire", "labels": ["OFF", "ON"]},
    {"name": "cheats", "labels": ["OFF", "ON"]}
]
```

| Where | How |
|---|---|
| Lobby (create popup) | `"Choice": {"name": "rule.difficulty", "options": ["NORMAL", "HARD", "EASY"]}` |
| Scripts | `if world.rule("difficulty") == "HARD": ...` |
| Server | `consoleRule: "cheats"` blocks console commands unless the room's `cheats` is `ON` |

The first label is the default. In solo mode every rule has its default.

## Adding a value

1. Add it to the table in the game file.
2. Set it from a script.
3. Show it in a scene.

Both sides read the same game file, so nothing else changes. Adding values changes the snapshot size: the
server checks at start that it still fits in a datagram.
