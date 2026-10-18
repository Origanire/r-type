# Design decisions

Each entry states the decision, why it was taken, its known costs and what it prepares.

## 1. A reusable engine separated from the game

**Decision.** `engine/` contains generic modules, each built as its own library; `game/` contains
everything specific to R-Type.

**Why.** Rendering, networking and game logic must be visible and decoupled subsystems. Making every
module a CMake library turns the dependency rules into build errors instead of conventions.

**Cost.** More targets and include paths to maintain.

**Prepares.** Extracting the engine as a standalone project and building a second game on it. A game
links only the modules it needs (link-time modularity).

## 2. Authoritative server, display-only client

**Decision.** The server simulates the whole game. Clients send held buttons and render snapshots.

**Why.** A single source of truth keeps every player in the same game and prevents client-side
cheating on lives, score or collisions.

**Cost.** Without prediction, a player's own ship reacts after one round trip.

**Prepares.** Client-side prediction with server reconciliation, which relies on an authoritative
server and a fixed-step simulation.

## 3. Entity Component System instead of an inheritance tree

**Decision.** Game objects are entities composed of data components, processed by systems.

**Why.** Composition avoids deep hierarchies and lets behaviors be combined freely. Components can be
added and removed at runtime: invulnerability after respawn is a component added then removed.

**Cost.** Logic is spread across systems; reading one object's behavior means reading several systems.

**Prepares.** New enemies, bosses or power-ups are new combinations of components plus, when needed,
one new system.

## 4. Bitmask ECS without templates, values owned by the game

**Decision.** `EntityManager` stores a 32-bit component mask per entity and never stores values.
Each game declares a world with one array per component type.

**Why.** The engine stays free of game types and of generic type-erasure code, so it is short,
readable and gives clear compiler errors. A game adds a component with one bit, one array and one
line in its factory.

**Cost.** Up to 32 component types per world (64 by switching the mask type). Every array has one
slot per entity, used or not.

**Prepares.** Systems only use `create`, `destroy` and `with`; the storage can later become a sparse
set or archetypes without changing the systems.

## 5. Fixed-capacity arrays and a free-index stack

**Decision.** Worlds reserve 1024 entities at startup. Freed rows are reused from a stack.

**Why.** No allocation during gameplay and direct indexed access. The protocol limits the replicated
world to 150 entities, so 1024 is a comfortable margin.

**Cost.** Memory is reserved even when unused (a few kilobytes per component).

## 6. Generational entity handles

**Decision.** An entity handle is an index plus a generation, incremented on destruction.

**Why.** Missiles and enemies are created and destroyed constantly and rows are reused immediately.
A stale handle must never affect the new entity of the same row.

**Prepares.** Safe references between entities (targets, homing missiles, attachments such as the
Force).

## 7. Systems behind an interface, run in explicit order

**Decision.** Systems implement `ISystem` and are owned by a `SystemManager` through `unique_ptr`.

**Why.** The engine runs game logic it does not know (polymorphism); ownership is automatic; the
whole simulation order is readable in one constructor.

**Prepares.** Adding a system is one class and one `add()` call.

## 8. Fixed-timestep simulation

**Decision.** The simulation advances in steps of exactly 1/60 s, with a catch-up limit of 0.25 s.

**Why.** Behavior must not depend on CPU speed; equal steps avoid tunnelling through collisions and
make the snapshot rate predictable.

**Prepares.** Interpolation (`FixedTimestep::alpha()`), prediction and replay, which all require a
deterministic step.

## 9. Networking hidden in the engine, on its own thread

**Decision.** `engine_net` wraps Asio privately and runs I/O on a background thread; the game sees
`send()` and a non-blocking `poll()`.

**Why.** The game loop never blocks on the network, and no game file depends on Asio.

**Prepares.** Replacing the network library, adding channels or metrics, inside `engine_net` only.

## 10. A binary UDP protocol with full snapshots

**Decision.** UDP, big-endian fields written one by one, 1200-byte maximum datagrams, a version byte,
session tokens, sequence numbers, strict validation, complete world snapshots at 20 Hz.

**Why.** UDP never stalls on a lost packet; complete snapshots make each packet self-sufficient, so
losses are corrected by the next one. Explicit encoding is identical on every compiler and OS.
Validation keeps malformed or forged packets from crashing or exhausting the server.

**Cost.** Full snapshots use more bandwidth than deltas (at most about 22 KB/s per client).

**Prepares.** Delta compression, quantization and reliable messages, introduced through the version
byte.

## 11. Rendering behind an interface

**Decision.** Game code draws through `IRenderer`; only `SfmlRenderer`, `Window` and `TextureManager`
use SFML.

**Why.** The server builds without any graphics library and can run headless; the game does not
depend on SFML types.

**Prepares.** Another backend, or a debug renderer, by implementing `IRenderer`.

## 12. Geometry-only physics module

**Decision.** `engine_physics` provides rectangles, overlap tests and pair detection; the game decides
what a collision means.

**Why.** Collision detection is reusable across games; collision rules are not.

**Cost.** Pair detection tests every pair (O(n²)), negligible for at most 150 entities.

**Prepares.** A spatial grid inside `findCollisions()` without touching game code.

## 13. Project-specific exceptions

**Decision.** Every error derives from `engine::EngineError`, with one class per module.

**Why.** The type and message prefix identify the failing module; one handler in each entry point
reports any project error and exits with code 84.

## 14. CMake and vcpkg, Linux and Windows

**Decision.** CMake generates the build; vcpkg in manifest mode with a pinned baseline provides
dependencies.

**Why.** The project builds on a clean machine without system-wide libraries, with the same library
versions for everyone, on Linux (GCC) and Windows (MSVC).

## Known limits

| Limit | Next step |
|---|---|
| `with()` scans every row | Sparse set or archetype storage |
| Full snapshots | Delta snapshots, quantization |
| No interpolation or prediction | Interpolation with `alpha()`, then prediction and reconciliation |
| O(n²) collision detection | Spatial grid |
| Events are best effort | Acknowledged reliable messages |
