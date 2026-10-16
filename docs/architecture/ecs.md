# Entity Component System

## Entities and component storage

An `Entity` contains an index and a generation. `Registry::spawnEntity()` allocates or reuses
an index. `killEntity()` clears every component, marks the slot dead and increments its generation.
An old handle therefore cannot modify a newly spawned entity at the same index.

`SparseArray<T>` uses `vector<optional<T>>`: the entity index selects a component slot;
a missing optional means that the entity does not have that component. Register each component
type before accessing it. Every `SparseArray<T>` implements the `IComponentArray` interface
(one virtual `erase(index)`); the registry stores them as `std::unique_ptr<IComponentArray>` keyed
by `std::type_index`, so `killEntity()` can clear an entity in every array without knowing their types.

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

## Systems and scheduling

A system is a plain function (or callable object) with the signature `void(SystemContext &)`.
The context gives access to the registry, the command buffer and the delta time. A system fetches
the component arrays it needs and visits the indices where all of them are present:

```cpp
void movementSystem(engine::ecs::SystemContext &context)
{
    auto &positions = context.registry.getComponents<rtype::Position>();
    auto &velocities = context.registry.getComponents<rtype::Velocity>();
    for (std::size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        if (!positions[i] || !velocities[i])
            continue;
        positions[i]->x += velocities[i]->x * context.dt;
        positions[i]->y += velocities[i]->y * context.dt;
    }
}
```

`Scheduler::addSystem(phase, system)` registers a system. `Scheduler::run(registry, dt)` runs
`Input`, `Update`, `Physics`, then `Render`, preserving registration order within each phase.
Deferred spawn/kill commands are applied after the complete run: use the command buffer when
changing entity membership while iterating.

## Current integration

The server's `Room` owns a registry and a scheduler running at 60 ticks per second:
`inputSystem` (Input phase) turns the received buttons into a velocity, then `movementSystem`
and `boundarySystem` (Physics phase) move the ships and keep them inside the world.
`EntityFactory` builds a player ship from its components (`Position`, `Velocity`, `Hitbox`,
`PlayerInput`, `Replicated`), and `collectNetworkEntities` turns `Position` + `Replicated`
into the entity list of a snapshot.

The client has separate logic and render schedulers. Local star positions are updated at a fixed
1/60-second timestep; rendering receives a zero delta time. Network ship positions are changed
only by snapshots. A map associates protocol `u16` IDs with local generational entity handles.
Neither registry nor scheduler is thread-safe; the network I/O worker never accesses them.
