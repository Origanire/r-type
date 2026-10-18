# Implementation roadmap

## Current milestone: networked movement

The current implementation supplies the first end-to-end multiplayer path. Completion here
means code exists and the listed behavior can be verified; it does not imply a complete R-Type game.

| ID | Task | Status | Acceptance criterion |
|---|---|---|---|
| N1 | Binary codec and validation | Implemented | Golden wire bytes, truncation and invalid fields are tested |
| N2 | UDP transport and bounded I/O queues | Implemented | Bad/oversized datagrams do not stop later reception |
| N3 | CONNECT/ACCEPT, tokens, four sessions | Implemented | Retries reuse a slot; fifth client is rejected |
| N4 | Disconnect, heartbeat and inactivity | Implemented | Explicit departures replicate; inactive sessions expire |
| N5 | Server-owned movement and snapshots | Implemented | Two clients receive the same ship positions |
| N6 | Graphical snapshot integration | Implemented; manual GUI check required | Two windows display each other's controlled ship |
| N8 | Protocol/architecture/build/run documentation | Written | Instructions and implementation limits match code |

## Next milestone: playable combat

Implement in this order. Keep authoritative logic on the server and use snapshots for truth.

| ID | Task | Depends on | Definition of done |
|---|---|---|---|
| G1 | Shared gameplay dimensions/rules and server ECS systems | N5 | Speed/size/timing constants are named; simulation can be tested without rendering |
| G2 | Shooting (implemented) | G1 | Held Fire creates kind-2 missiles at a server-limited cadence; both clients see them; offscreen missiles are removed |
| G3 | Basic enemies | G1 | Kind-3 enemies spawn on the right and move left; rendering uses the enemy assets |
| G4 | Collision and damage (implemented) | G2, G3 | Reusable AABB tests; each impact is resolved once; destroyed entities disappear on every client |
| G5 | Lives, score and death (implemented on the server, HUD missing) | G4 | Server updates player fields; deaths and scoring match on all clients; HUD displays authoritative values |
| G6 | Waves and game states | G3, G5 | Configured waves progress through waiting/playing/game-over; empty sessions do not keep a combat round alive |
| G7 | Enemy shots | G4, G5 | Kind-4 missiles damage players using the same collision rules |
| G8 | Animation and audio feedback | G2-G7 | Effects are cosmetic; losing an event never changes gameplay truth |

Suggested implementation boundaries:

- `engine/physics`: generic AABB overlap and collision helpers, no R-Type rules.
- `game/shared`: component/data types (`Hitbox`, `Health`, `Projectile`, `Enemy`, `Score`), rules and assets/kind mapping.
- `game/server`: Input, Movement, Shooting, Spawn, Collision, Damage, Cleanup and Snapshot systems.
- `game/client`: sprite/animation mapping, HUD and optional snapshot interpolation.
- Enforce the RFC's 150-entity limit before spawning. Never omit arbitrary live entities from a full snapshot:
  absence means destruction to clients. If capacity is exhausted, defer/refuse spawns.

Version 1 already has entity kinds for missiles/enemies and player lives/score. If a feature needs
new wire fields, update the RFC and codec tests first and bump the protocol version for incompatible changes.

## Quality work alongside gameplay

1. Add ECS lifetime/command-buffer tests and simulation tests for shooting/collisions/scoring.
2. Exercise loss, delay, duplication and reordering on a separate test interface; document measurements.
3. Native Windows build and Linux/Windows crossplay were validated on 2026-10-13; keep SFML out of server dependencies.
4. Add a CI build/test job with a vcpkg binary cache and document branching/review practices.
5. Finish architecture examples, comparative study and accessibility design; add remappable saved controls.

Each row can become one issue/feature branch/PR targeting `dev`, following the coding conventions.
Do not estimate project completion from file count: the mandatory combat loop, resilience and usability
still account for most of the remaining work. Advanced tracks are a later milestone after the first playable game.
