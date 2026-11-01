# Engine modules

Each module lives in `engine/<module>/` with `include/` and `src/` folders, has its own namespace
(`engine::<module>`, except `engine` for core) and is built as its own static library. A game links
only the modules it needs.

| Module | Provides | Depends on |
|---|---|---|
| `engine_core` | `EngineError` (base of every project exception), `CoreError`, `Clock` (steady clock stopwatch), `FixedTimestep` (fixed-step accumulator with catch-up limit) | — |
| `engine_ecs` | `Entity` (index + generation), `EntityManager` (component masks, free list, queries), `ISystem`, `SystemManager`, `EcsError` | core |
| `engine_physics` | `Aabb`, `overlaps()`, `Body`, `CollisionPair`, `findCollisions()` | — |
| `engine_net` | `UdpTransport` (IPv4 UDP socket with a background I/O thread, bounded queues), `Endpoint`, `Datagram`, big-endian `ByteWriter` / `ByteReader`, `NetError` | core, Asio (private) |
| `engine_render` | `Window`, `IRenderer`, `SfmlRenderer`, `TextureManager`, `Color` / `Vector2` / `IntRect`, `RenderError` | core, SFML Graphics |
| `engine_input` | `Action`, `KeyBindings` (remappable), `InputManager` (focus-aware sampling, action bitmask) | core, SFML Window |
| `engine_audio` | Not implemented yet | — |
| `engine_ui` | Not implemented yet | — |

## Rules

- No engine file includes a file from `game/`.
- Third-party libraries stay inside the module that wraps them: Asio is a private dependency of
  `engine_net`, and SFML only appears in `engine_render` and `engine_input`.
- The engine never stores game data. The ECS records which components an entity has; the game owns
  the component values (see [ECS](ecs.md)).
- `engine_physics` does not depend on the ECS: it works on plain rectangles, so any game, with any
  storage, can use it.

## Errors

```text
std::exception
└── engine::EngineError          engine/core
    ├── engine::CoreError        engine/core
    ├── engine::ecs::EcsError    engine/ecs
    ├── engine::net::NetError    engine/net
    ├── engine::render::RenderError  engine/render
    └── rtype::GameError         game/shared
```

Every message starts with the name of the class that raised it. Both entry points catch
`EngineError`, then `std::exception`, print the message and return 84.

## Time

`Clock::restart()` returns the seconds elapsed since the previous call, measured with
`std::chrono::steady_clock` (never affected by system clock changes). `FixedTimestep` accumulates
this time and releases it in fixed steps:

```cpp
timestep.addTime(clock.restart());
while (timestep.step())
    update(timestep.stepSeconds());
```

The simulation therefore advances by identical `dt` values regardless of CPU speed. Each frame adds
at most 0.25 s, so a long freeze cannot trigger an endless catch-up.
