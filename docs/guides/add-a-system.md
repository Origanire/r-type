# Add a system

Read the [ECS overview](../architecture/ecs.md) first. A system describes one behavior over
entities having a required set of components. Gameplay systems run on the server; client systems
only handle presentation (stars, effects, rendering).

1. Declare the system in the module's `include/systems/` directory, for example
   `void movementSystem(engine::ecs::SystemContext &context);`.
2. Implement it in `src/systems/`: fetch the component arrays with
   `context.registry.getComponents<T>()` and visit the indices where all of them are present.
3. Register it in a phase and add its `.cpp` to `CMakeLists.txt`:

```cpp
m_scheduler.addSystem(engine::ecs::Phase::Physics, engine::physics::movementSystem);
```

Scale simulation changes by `context.dt`; never move a fixed distance per rendered frame.
Use `context.commands.spawn(...)` and `kill(...)` for structural changes during iteration. Spawn
callbacks receive `(Registry &, const Entity &)`, where components can be initialized after iteration.
Commands execute after the complete scheduler run, not immediately between phases.

For authoritative gameplay, run the system on the server (in `Room`), replicate results through
snapshots, and let the client render. Client systems must not decide scores, collisions or deaths.

To react to collisions, register an `engine::physics::CollisionSystem` with a listener. It is
called once per overlapping pair (`Collision{first, second}`), after every pair has been found,
so the listener may queue kills through the command buffer or game events:

```cpp
m_scheduler.addSystem(engine::ecs::Phase::Physics, engine::physics::CollisionSystem(
    [this](const engine::physics::Collision &collision) { /* damage, destroy... */ }));
```
