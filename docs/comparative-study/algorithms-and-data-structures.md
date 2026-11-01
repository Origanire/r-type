# Algorithms and data structures

## Object model

| Option | Strengths | Weaknesses |
|---|---|---|
| Inheritance hierarchy (`GameObject` → `Ship`, `Enemy`…) | Familiar | Deep, rigid trees; behaviors cannot be combined or changed at runtime |
| **Entity Component System** | Composition, runtime changes, systems process homogeneous data | Behavior spread across systems |

**Choice:** ECS, as recommended by the subject. Example of runtime composition: invulnerability is a
component added on respawn and removed after 2 s.

## ECS storage

| Variant | How it works | Strengths | Weaknesses |
|---|---|---|---|
| **Bitmask + one array per component** (ours) | One mask per entity; arrays indexed by entity index | Very simple, O(1) access, no allocation in gameplay | Arrays sized for every entity; queries scan all rows |
| Sparse set (e.g. EnTT) | Dense packed arrays plus a sparse index per component | Fast iteration over large sets, compact | More complex, usually template-based |
| Archetypes (e.g. Unity DOTS, flecs) | Entities grouped by exact component combination | Fastest iteration on huge worlds | Moving entities between tables on every change, complex |
| `vector<optional<T>>` per component with type erasure | One generic array per type, looked up by type | Generic | Templates and type erasure, harder to read |

**Choice:** bitmask + arrays. The world holds at most about 150 replicated entities plus 205 stars on
the client; a full scan of 1024 rows costs a few microseconds per query. The engine stays free of game
types and templates. Systems only use `create`, `destroy` and `with`, so the storage can evolve later.

## Entity handles

| Option | Risk |
|---|---|
| Raw pointers to objects | Dangling pointers after destruction |
| Plain integer ids | An old id silently targets the new entity reusing the slot |
| **Generational index** (index + generation) | A stale handle is detected because the generation differs |

Freed indices are reused from a **stack** (last freed, first reused), which keeps recently used rows
hot in cache. The generation makes this immediate reuse safe.

## Time step

| Option | Effect |
|---|---|
| Per-frame movement | Speed depends on CPU speed — forbidden by the subject |
| Variable `dt` | Speed is correct but results depend on frame timing; large steps can skip collisions |
| **Fixed step with accumulator** | Identical steps of 1/60 s, bounded catch-up (0.25 s per frame) |

## Collision detection

| Shape test | Use |
|---|---|
| **Axis-aligned bounding boxes** | Four comparisons; matches rectangular sprites |
| Circles | Cheap, poor fit for long missiles |
| Separating axis theorem | Rotated polygons; unnecessary here |

| Broad phase | Cost |
|---|---|
| **All pairs** | O(n²); about 11,000 tests for 150 entities, negligible |
| Uniform grid / quadtree | O(n) on average; worthwhile for thousands of entities |

`findCollisions()` gathers every overlapping pair first, then the game resolves them; checking
`isAlive` before each resolution guarantees an entity is destroyed at most once per tick.

## Network data structures

- Snapshots are vectors of fixed 7-byte records; the 150-entity cap keeps a snapshot under 1200 bytes.
- Sequence numbers use modular comparison (`(a - b) mod 2^32 < 2^31`) so counters can wrap.
- The receive queue is swapped out in one operation, keeping the mutex section minimal.

## Design patterns

| Pattern | Where |
|---|---|
| Entity Component System | `engine_ecs`, all game logic |
| Interface / strategy | `ISystem`, `IRenderer` |
| Private implementation (pimpl) | `UdpTransport::Impl` hides Asio |
| Factory | `EntityFactory` (`createPlayerShip`, `createPlayerMissile`, `createBydo`) |
| Callback / observer | Collision listener, `nextNetworkId`, `canSpawn` |
| State machine | Game state (waiting, playing, game over), client connection |
| RAII | `unique_ptr` systems, transport thread joined in the destructor |
