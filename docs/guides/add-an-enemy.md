# Add an enemy

Two cases: a new **movement pattern** for the existing Bydo, or a new **kind** of enemy with its own
sprite.

## A new movement pattern

1. Add a value to `EnemyPattern` in `game/server/include/components/Enemy.hpp` and increase
   `ENEMY_PATTERN_COUNT`.
2. Handle it in the `switch` of `EnemyMovementSystem::update`: set the entity's velocity from its
   `Enemy` data (`age`, `baseY`) and its position.
3. If the pattern needs a specific starting velocity, set it in `createBydo`.

`SpawnSystem` picks patterns at random among `ENEMY_PATTERN_COUNT`, so the new pattern appears
without further changes.

## A new kind of enemy

### Server

1. Add its constants (size, speed, health, score) in `game/shared/include/GameConstants.hpp`.
2. Add a kind value in `game/shared/include/RtypeProtocol.hpp`, using the values reserved by the
   [protocol](../protocol.md) when they fit, and document it there.
3. Write a factory function next to `createBydo` in `EntityFactory.cpp`. Give it `Position`,
   `Velocity`, `Hitbox`, `Enemy`, `Health` and `Replicated` with the new kind. Any entity with `Enemy`
   is already handled by collisions, scoring and offscreen removal.
4. Call it from `SpawnSystem`, or from a new spawning system.

### Client

1. Add a `SpriteSheet` constant in `game/client/include/SpriteSheet.hpp` pointing to the image in
   `assets/sprites/enemies/`.
2. Load its texture in the `Client` constructor.
3. Add a `create<Enemy>()` function and handle the new kind in `Client::applySnapshot`.

Unknown kinds are ignored by older clients, so they keep working.

### Example: an enemy that also shoots

Add `Shooter` to the enemy in its factory, then write an `EnemyShootingSystem` that spawns enemy
missiles with a new kind. A collision rule in `Room::onCollision` makes those missiles damage ships.
