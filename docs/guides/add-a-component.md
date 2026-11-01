# Add a component

A component is plain data; behavior goes in a system. Example: a `Shield` that absorbs hits on the
server.

## 1. Declare the struct

Server-only data goes in `game/server/include/components/`, client-only data in
`game/client/include/components/`, data needed by both in `game/shared/include/components/`.

```cpp
#pragma once

namespace rtype::server {

struct Shield
{
    int charges = 0;
};

} // namespace rtype::server
```

## 2. Give it a bit and an array in the world

In `game/server/include/ServerWorld.hpp`, use the next free bit and add an array:

```cpp
inline constexpr engine::ecs::ComponentMask SHIELD = 1 << 11;

std::vector<Shield> shields = std::vector<Shield>(MAX_ENTITIES);
```

A world holds at most 32 component types.

## 3. Attach it

In a factory function, write the value **before** setting the bit:

```cpp
world.shields[i] = Shield{2};
world.entities.addComponents(ship, POSITION | VELOCITY | SHIELD);
```

At runtime, `addComponents` and `removeComponents` can add or remove it from an existing entity.

## 4. Use it

Query it in a system with `m_world.entities.with(SHIELD | ...)`, or check one entity with
`m_world.entities.hasComponents(entity, SHIELD)`.

## Checklist

- Unique header name, `#pragma once`, no function body in the header.
- If clients must see the data, update the snapshot format, the codec and the
  [protocol](../protocol.md) together; never send a struct as raw memory.
