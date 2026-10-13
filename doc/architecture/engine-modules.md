# Engine modules

This inventory describes code actually compiled by CMake.

| Module | Existing responsibilities | Remaining work |
|---|---|---|
| `engine_core` | `EngineError`, `Clock`, `FixedTimestep` | Logger, configuration, paths, event bus |
| `engine_ecs` | Generational entities, typed component arrays, registry, zipped iteration, phase scheduler, deferred commands | Dedicated ECS tests, usage documentation extensions |
| `engine_net` | IPv4 UDP socket, private Asio implementation, background I/O, bounded inbox/outbox | Metrics, IPv6/DNS if needed, stronger connection authentication |
| `engine_render` | Window/events, renderer interface, SFML implementation, named textures | Fonts, animation, palette support |
| `engine_input` | Keyboard-to-action bindings, focus handling, action bitmask | Saved bindings, settings UI, controller support |
| `engine_physics` | No standalone library yet | Reusable AABB and collision systems |
| `engine_audio` | Not implemented | Sound/music loading and playback |
| `engine_ui` | Not implemented | Menus, widgets, settings, HUD |

## Game modules

- `rtype_shared` is a compiled library containing the binary codec, plus shared headers
  (`Position`, `Velocity`, world dimensions, protocol types, CLI port parsing).
- `Server` owns four optional sessions, an ECS registry of ship positions, input freshness,
  fixed-tick movement and snapshots. It links no SFML rendering/input/audio library.
- `NetworkClient` owns connection retries, the server endpoint, token, outgoing sequence,
  periodic inputs/heartbeats, snapshot ordering and timeout detection.
- `Client` owns presentation, converts snapshots to ECS entities and retains local star entities.

`MovementSystem`, `PlayerControlSystem` and `ScreenClampSystem` are retained local-prototype
helpers, but are no longer scheduled for network ships. Reusing them for prediction later
requires reconciliation with server authority.
