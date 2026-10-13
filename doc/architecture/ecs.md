# Entity Component System

## Entities and component storage

An `Entity` contains an index and a generation. `Registry::spawnEntity()` allocates or reuses
an index. `killEntity()` clears every component, marks the slot dead and increments its generation.
An old handle therefore cannot modify a newly spawned entity at the same index.

`SparseArray<T>` uses `vector<optional<T>>`: the entity index selects a component slot;
a missing optional means that the entity does not have that component. Register each component
type before accessing it. The registry stores typed arrays behind `std::any`/`std::type_index`.

```cpp
engine::ecs::Registry registry;
registry.registerComponent<rtype::Position>();
const auto ship = registry.spawnEntity();
registry.addComponent(ship, rtype::Position{200.f, 540.f});
auto &position = *registry.getComponents<rtype::Position>()[ship];
position.x += 10.f;
registry.killEntity(ship);
```

Direct `SparseArray` access is low-level: mutable indexing may grow the array. Prefer registry
operations for adding/removing components so that entity liveness is checked.

## Iteration and systems

`Zipper` traverses matching occupied entries in several component arrays; `IndexedZipper`
also returns their index. Systems receive a `SystemContext` with the registry, command buffer
and delta time, followed by their required component arrays.

```cpp
void move(engine::ecs::SystemContext &context,
    engine::ecs::SparseArray<rtype::Position> &positions,
    engine::ecs::SparseArray<rtype::Velocity> &velocities)
{
    for (auto [position, velocity] : engine::ecs::Zipper(positions, velocities)) {
        position.x += velocity.x * context.dt;
        position.y += velocity.y * context.dt;
    }
}
```

The scheduler runs `Input`, `Update`, `Physics`, then `Render`, preserving registration order
within each phase. Deferred spawn/kill commands are applied after the complete scheduler run.
Use the command buffer when changing entity membership while iterating. The registry's older
`addSystem`/`runSystems` interface also exists, but the client uses the phase scheduler.

## Current integration

The client has separate logic/render schedulers. Local star positions are updated at a fixed
1/60-second timestep; rendering receives a zero delta time. Network ship positions are changed
only by snapshots. A map associates protocol `u16` IDs with local generational entity handles.

The server uses its own registry for ship positions and a fixed-tick loop. Its current movement
code is in `Server::step`; gameplay systems should be extracted as shooting/collisions arrive.
Neither registry nor scheduler is thread-safe. The I/O worker never accesses them.
