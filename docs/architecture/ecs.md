# Entity Component System

The engine provides a bitmask-based ECS with generational entities. The engine tracks which
components each entity has; the game owns the component values in plain arrays.

## Concepts

| Concept | In the code |
|---|---|
| Entity | `engine::ecs::Entity`: an index (row) and a generation |
| Component | A plain struct in `game/`, e.g. `Position { float x; float y; }` |
| Component bit | A game constant, e.g. `POSITION = 1 << 0` |
| World | A game struct holding an `EntityManager` and one `std::vector` per component |
| System | A class implementing `engine::ecs::ISystem::update(float dt)` |

Think of a spreadsheet: each row is an entity, each column a component type. `EntityManager` holds
the column of ticked boxes; the world holds the value columns.

## Entity

An `Entity` is a small value type (`index()`, `generation()`). Only `EntityManager` can create one.
When an entity is destroyed, the generation of its row is incremented, so any copy of the old handle
becomes invalid even if the row is immediately reused.

## EntityManager

```cpp
using ComponentMask = std::uint32_t;

class EntityManager
{
public:
    explicit EntityManager(std::size_t capacity);
    Entity create();
    void destroy(const Entity &entity);
    bool isAlive(const Entity &entity) const;
    Entity entityAt(std::size_t index) const;
    void addComponents(const Entity &entity, ComponentMask components);
    void removeComponents(const Entity &entity, ComponentMask components);
    bool hasComponents(const Entity &entity, ComponentMask components) const;
    std::vector<std::size_t> with(ComponentMask components) const;
};
```

| Function | Behavior |
|---|---|
| `create()` | Takes the most recently freed row, marks it alive with an empty mask. Throws `EcsError` when full. |
| `destroy(e)` | Clears the mask, increments the generation, frees the row. No effect on a dead entity. |
| `isAlive(e)` | True if the row is alive and has the same generation as `e`. |
| `entityAt(i)` | Handle of the living entity at row `i`. Throws `EcsError` if the row is dead. |
| `addComponents(e, bits)` | `mask |= bits`. Throws `EcsError` on a dead entity. |
| `removeComponents(e, bits)` | `mask &= ~bits`. No effect on a dead entity. |
| `hasComponents(e, bits)` | `(mask & bits) == bits` on a living entity. |
| `with(bits)` | Copy of the rows of living entities having every requested bit. |

Because `with()` returns a copy, systems may create or destroy entities while iterating; no deferred
command queue is needed. A mask holds up to 32 component types per world.

## Worlds

Each program defines its own world. Every component array has the same size as the
`EntityManager`, so `array[entity.index()]` is always valid.

```cpp
inline constexpr std::size_t MAX_ENTITIES = 1024;
inline constexpr engine::ecs::ComponentMask POSITION = 1 << 0;
inline constexpr engine::ecs::ComponentMask VELOCITY = 1 << 1;

struct ServerWorld
{
    engine::ecs::EntityManager entities{MAX_ENTITIES};
    std::vector<Position> positions = std::vector<Position>(MAX_ENTITIES);
    std::vector<Velocity> velocities = std::vector<Velocity>(MAX_ENTITIES);
};
```

| World | Components |
|---|---|
| `ServerWorld` | Position, Velocity, Hitbox, PlayerInput, Replicated, Shooter, Projectile, Enemy, Health, PlayerShip, Invulnerable |
| `ClientWorld` | Position, Sprite, Star, ShipTilt |

## Creating an entity

Always in three steps: take a row, write **every** component it will have (rows are recycled without
being cleared), then set the bits so that systems see a complete entity.

```cpp
engine::ecs::Entity ship = world.entities.create();
world.positions[ship.index()] = Position{PLAYER_SPAWN_X, spawnY};
world.velocities[ship.index()] = Velocity{};
world.entities.addComponents(ship, POSITION | VELOCITY);
```

The server keeps these recipes in `EntityFactory` (`createPlayerShip`, `createPlayerMissile`,
`createBydo`). Components can also be added or removed later: a respawned ship receives
`INVULNERABLE`, which `InvulnerabilitySystem` removes when its timer expires.

## Systems

```cpp
class MovementSystem : public engine::ecs::ISystem
{
public:
    explicit MovementSystem(ServerWorld &world);
    void update(float dt) override;
private:
    ServerWorld &m_world;
};

void MovementSystem::update(float dt)
{
    for (std::size_t i : m_world.entities.with(POSITION | VELOCITY)) {
        m_world.positions[i].x += m_world.velocities[i].x * dt;
        m_world.positions[i].y += m_world.velocities[i].y * dt;
    }
}
```

`SystemManager` owns systems as `std::unique_ptr<ISystem>` and runs them in insertion order. It
rejects null systems and negative `dt` with `EcsError`.

## Threading

The ECS is not thread-safe and does not need to be: each world is only used by the main thread of
its program. The network thread never touches it.
