# Add a system

Read the [ECS overview](../architecture/ecs.md) first. A system describes one behavior over
entities having a required set of components. It must not depend on rendering to run on the server.

1. Declare a function or callable in the appropriate module's `include/` directory.
2. Implement it in `src/`, with `SystemContext &` and the required component arrays.
3. Use `Zipper` for matching components or `IndexedZipper` if entity indices are needed.
4. Register it in a phase and add its `.cpp` to CMake:

```cpp
scheduler.addSystem<rtype::Position, rtype::Velocity>(
    engine::ecs::Phase::Physics, movementSystem);
```

The existing client `movementSystem` is an example of the required function signature, but it
is no longer scheduled for network ships. Server movement currently lives in `Server::step`;
extract reusable simulation rules there before adding a gameplay scheduler.

Scale simulation changes by `context.dt`; never move a fixed distance per rendered frame.
Use `context.commands.spawn(...)` and `kill(...)` for structural changes during iteration. Spawn
callbacks receive `(Registry &, const Entity &)`, where components can be initialized after iteration.
Commands execute after the complete scheduler run, not immediately between phases.

For authoritative gameplay, run the system on the server, replicate results via snapshots,
and let the client render. Client systems may handle stars, effects or future interpolation;
they must not decide scores, collisions or deaths. Add a behavior test that checks the rule rather
than merely repeating its implementation.
