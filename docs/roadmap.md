# Roadmap

## Part 1 — prototype

| Feature | Status |
|---|---|
| Engine modules: core, ECS, physics, network, rendering, input | Done |
| Binary UDP protocol, sessions, heartbeats, timeouts, crash notification | Done |
| Server-authoritative movement, 60 Hz simulation, 20 Hz snapshots | Done |
| Scrolling starfield | Done |
| Four distinguishable player ships, tilt animation | Done |
| Player missiles | Done |
| Bydos spawning at random on the right (straight, sine, zigzag) | Done |
| Collisions, damage, lives, respawn with invulnerability, score, game over and new round | Done |
| Linux build | Done |
| Windows build (MSVC) | Done |
| Linux ↔ Windows cross-play | To verify |
| Enemy missiles (kind 4) | To do |
| Sound effects and music (`engine_audio`) | To do |
| HUD: lives, score, player numbers | To do |
| In-game accessibility settings | To do — see [Accessibility](accessibility.md) |
| Documentation website published | To do (`mkdocs gh-deploy`) |
| Git tag on the delivered commit | To do before the defense |

## Part 2 — candidate topics

| Track | Topics |
|---|---|
| Advanced architecture | Standalone engine project, second sample game, audio and UI modules, resource management, developer console, metrics |
| Advanced gameplay | Enemy variety, bosses, the Force, power-ups, levels, data-driven content in JSON, content creation guides |
| Advanced networking | Interpolation, client prediction with reconciliation, delta snapshots, reliable messages, bandwidth measurements |

Each feature is tracked as a GitHub issue and developed on its own branch.
