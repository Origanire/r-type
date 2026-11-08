# Roadmap

## Done

| Area | Features |
|---|---|
| Engine | Modules with private third-party dependencies, ECS with sparse arrays, data-driven prefabs, scenes and systems, standalone CMake with `engine_add_game`, generic launchers |
| Scripting | Embedded Python with hooks, per-entity error isolation, full world and entity API, console scripts |
| Network | Binary UDP protocol v7, sessions, heartbeats, timeouts, rooms with codes and rules, kicks, console messages, sounds |
| Client | Menus, lobby, room and game scenes in JSON, HUD bound to values, pause menu, F1 console, F3 metrics, loading screen, gamepad |
| Solo mode | `LocalClient`, `LOCAL` games |
| R-Type | Three themed stages (a warship corridor, a giant vessel hunting the players), three bosses, power-ups, the Force (front or back, free, super-charged shots), charged shot, level choice, difficulty, friendly fire and bonus rules |
| Second game | Skyhop, solo, Kenney CC0 assets |
| Platforms | Linux, Windows, LAN play |
| Documentation | Architecture, references, guides, RFC, comparative study, accessibility, team processes |
| Settings screen | Volumes, text size, toggle fire, labels, dead zone, key and button rebinding, saved per player |
| Plugins | Systems in shared libraries, loaded from the game file and reloaded while the game runs |
| Tooling | GitHub Actions CI (format check, Linux and Windows builds, cached dependencies), `clang-format` |

## Next

| Item | Track | Why |
|---|---|---|
| Colorblind palettes, option to disable shake and flashes | Accessibility | See [Accessibility](accessibility.md) |
| Client-side prediction and interpolation | Network | Remove the round-trip delay on the local ship |
| Delta snapshots | Network | Less bandwidth, more entities |
| Generic message bus between systems and scripts | Architecture | Fewer dedicated event queues |
| Automated tests of the simulation and the codec | Architecture | See [Testing](guides/testing.md) |

Each item becomes a GitHub issue and is developed on its own branch ([Git workflow](contributing/git-workflow.md)).
