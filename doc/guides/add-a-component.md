# Add a component

A component is data; put behavior in a system. Use a unique header name, `#pragma once`,
and the naming/namespace rules from [conventions](../conventions.md).

1. Put reusable engine data in its engine module; R-Type gameplay data goes in
   `game/shared/include/components/`, presentation-only data in `game/client/include/components/`.
2. Define a small struct. This example is a proposed future component, not an implemented damage system:

```cpp
#pragma once
namespace rtype {
struct Health
{
    int current = 1;
    int maximum = 1;
};
}
```

3. Call `registry.registerComponent<rtype::Health>()` before spawning entities that use it.
4. Attach data with `registry.addComponent(entity, rtype::Health{3, 3})`.
5. Register a system requiring that component. For entity death during iteration, use deferred
   `context.commands.kill(entity)` instead of mutating storage immediately.

Gameplay components belong to the server's simulation. Adding a shared C++ type does not change
network serialization: explicitly update snapshots/codec/RFC if clients need new fields. Never serialize
the struct's memory. Add a new `.cpp` to CMake when the component requires non-template implementation.
