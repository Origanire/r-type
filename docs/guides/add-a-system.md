# Add a system

A system is one behavior applied to every entity having a given set of components. Gameplay
systems run on the server; client systems only handle presentation.

## 1. Declare the class

`game/server/include/systems/ShieldSystem.hpp`:

```cpp
#pragma once

#include "ISystem.hpp"
#include "ServerWorld.hpp"

namespace rtype::server {

class ShieldSystem : public engine::ecs::ISystem
{
public:
    explicit ShieldSystem(ServerWorld &world);

    void update(float dt) override;

private:
    ServerWorld &m_world;
};

} // namespace rtype::server
```

## 2. Implement it

`game/server/src/systems/ShieldSystem.cpp`:

```cpp
#include "ShieldSystem.hpp"

namespace rtype::server {

ShieldSystem::ShieldSystem(ServerWorld &world) : m_world(world)
{
}

void ShieldSystem::update(float)
{
    for (std::size_t i : m_world.entities.with(SHIELD)) {
        if (m_world.shields[i].charges <= 0)
            m_world.entities.removeComponents(m_world.entities.entityAt(i), SHIELD);
    }
}

} // namespace rtype::server
```

Rules inside `update`:

- Scale every change over time by `dt`.
- Creating or destroying entities while iterating is safe: `with()` returns a copy.
- Use `entityAt(i)` to get a handle when an engine call needs one.

## 3. Register it

Add it in the `Room` constructor at the right place in the order, and add the `.cpp` to the
`r-type_server` target in `CMakeLists.txt`:

```cpp
m_systems.add(std::make_unique<ShieldSystem>(m_world));
```

If the system needs something owned by `Room` (network ids, game state), pass a callback to its
constructor, as `ShootingSystem` and `SpawnSystem` do.

## Client systems

Client systems follow the same pattern with `ClientWorld`. Register them in `Client::registerSystems`:
`m_logic` for fixed-step logic, `m_render` for drawing.
