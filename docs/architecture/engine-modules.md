# Engine modules

This inventory describes code actually compiled by CMake.

| Module | Existing responsibilities | Remaining work |
|---|---|---|
| `engine_core` | `EngineError`, `Clock`, `FixedTimestep` | Logger, configuration, paths, event bus |
| `engine_ecs` | Generational entities, typed component arrays, registry, phase scheduler, deferred commands | Dedicated ECS tests, usage documentation extensions |
| `engine_net` | IPv4 UDP socket, private Asio implementation, background I/O, bounded inbox/outbox, big-endian `ByteWriter`/`ByteReader`, `NetError` | Metrics, IPv6/DNS if needed, stronger connection authentication |
| `engine_render` | Window/events, renderer interface, SFML implementation, named textures | Fonts, animation, palette support |
| `engine_input` | Keyboard-to-action bindings, focus handling, action bitmask | Saved bindings, settings UI, controller support |
| `engine_physics` | No standalone library yet | Reusable AABB and collision systems |
| `engine_audio` | Not implemented | Sound/music loading and playback |
| `engine_ui` | Not implemented | Menus, widgets, settings, HUD |

## Game modules

- `rtype_shared` is a compiled library containing the binary codec (built on `ByteWriter` and
  `ByteReader`), plus shared headers: components (`Position`, `Velocity`, `Hitbox`), game
  constants (`GameConstants.hpp`), protocol types and constants, `GameError` and CLI port parsing.
- The server is split in two. `Server` owns the UDP transport and the four optional sessions
  (connection, tokens, sequences, timeouts, events). `Room` owns the game world: an ECS registry,
  a 60 Hz scheduler with `inputSystem`, `movementSystem` and `boundarySystem`, and the
  `EntityFactory`. It links no SFML rendering/input/audio library.
- `NetworkClient` owns connection retries, the server endpoint, token, outgoing sequence,
  periodic inputs/heartbeats, snapshot ordering and timeout detection.
- `Client` owns presentation, converts snapshots to ECS entities and retains local star entities.
