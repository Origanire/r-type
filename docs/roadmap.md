# Roadmap

## Done

| Area | Features |
|---|---|
| Engine | Modules with private third-party dependencies, ECS with sparse arrays, data-driven prefabs, scenes and systems, standalone CMake with `engine_add_game`, generic launchers |
| Scripting | Embedded Python with hooks, per-entity error isolation, full world and entity API, console scripts |
| Network | Binary UDP protocol v7, sessions, heartbeats, timeouts, rooms with codes and rules, kicks, console messages, sounds |
| Client | Menus, lobby, room and game scenes in JSON, HUD bound to values, pause menu, F1 console, F3 metrics, loading screen, gamepad |
| Solo mode | `LocalClient`, `LOCAL` games |
| R-Type | Three stages, waves, three bosses, power-ups, the Force, charged shot, terrain, difficulty and friendly fire rules |
| Second game | Skyhop, solo, Kenney CC0 assets |
| Platforms | Linux, Windows, LAN play |
| Documentation | Architecture, references, guides, RFC, comparative study, accessibility, team processes |
| Tooling | GitHub Actions CI (format check, Linux and Windows builds, cached dependencies), `clang-format` |

## Next

| Item | Track | Why |
|---|---|---|
| Settings menu in game | Gameplay / accessibility | Change keys, volumes and text size without editing a file |
| Colorblind palettes, option to disable shake and flashes | Accessibility | See [Accessibility](accessibility.md) |
| Client-side prediction and interpolation | Network | Remove the round-trip delay on the local ship |
| Delta snapshots | Network | Less bandwidth, more entities |
| Generic message bus between systems and scripts | Architecture | Fewer dedicated event queues |
| Plugins (systems loaded from shared libraries) | Architecture | Engine extensions without rebuilding |
| Automated tests of the simulation and the codec | Architecture | See [Testing](guides/testing.md) |

Each item becomes a GitHub issue and is developed on its own branch ([Git workflow](contributing/git-workflow.md)).
