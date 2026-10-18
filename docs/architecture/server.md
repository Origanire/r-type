# Server

`r-type_server` is the authority: it runs every gameplay rule and tells clients what happens.
It has no window and links no rendering, input or audio library.

## Structure

| Class | Responsibility | Files |
|---|---|---|
| `main` | Parses `-p`, installs `SIGINT`/`SIGTERM` handlers, runs the fixed 60 Hz loop | `game/server/src/main.cpp` |
| `Server` | UDP transport, four player sessions, tokens, timeouts, event broadcast, snapshots | `Server.hpp`, `Server.cpp` |
| `Room` | The game world: ECS, systems, lives, score, respawns, game state, collision outcomes | `Room.hpp`, `Room.cpp` |
| `EntityFactory` | Recipes for ships, missiles and Bydos | `EntityFactory.hpp`, `EntityFactory.cpp` |
| `GameEvents` | Queue of gameplay events raised during a tick | `GameEvents.hpp`, `GameEvents.cpp` |
| `Snapshot` | Builds the replicated entity list, counts replicated entities | `Snapshot.hpp`, `Snapshot.cpp` |

`Server` deals with the network and players; `Room` deals with the game. `Room` does not know about
endpoints or packets, which keeps the simulation independent from the transport.

## Main loop

```cpp
while (running) {
    server.poll();
    timestep.addTime(clock.restart());
    while (timestep.step())
        server.tick(timestep.stepSeconds());
    std::this_thread::sleep_for(IDLE_DELAY);
}
server.shutdown();
```

`poll()` never blocks: datagrams are received by the network thread and handed over in a queue.
The game keeps running frame after frame whether clients send data or not.

## Sessions

A session is created by `CONNECT` and confirmed by the first valid `INPUT` or `HEARTBEAT`, which
spawns the player's ship and broadcasts `PLAYER_JOINED`. A session ends on `DISCONNECT`
(`PLAYER_LEFT`, reason 1) or after 5 s of silence (`PLAYER_LEFT`, reason 2: the client crashed or lost
its connection). The other players keep playing. Details are in the [protocol](../protocol.md).

## Components

| Component | Data | Used for |
|---|---|---|
| `Position`, `Velocity`, `Hitbox` | shared structs | Movement, bounds, collisions |
| `PlayerInput` | held buttons | Converted into velocity and fire |
| `Replicated` | network id, kind | Sent in snapshots |
| `Shooter` | cooldown | Fire rate limit |
| `Projectile` | owner slot | Score attribution |
| `Enemy` | pattern, base height, age | Bydo movement |
| `Health` | points | Damage |
| `PlayerShip` | slot | Link between a ship and its player |
| `Invulnerable` | remaining time | Protection after respawn |

## Systems, in execution order

| System | Entities | Effect |
|---|---|---|
| `InputSystem` | PlayerInput, Velocity | Arrow buttons → velocity |
| `ShootingSystem` | PlayerInput, Shooter, Position, PlayerShip | Fire button → missile, every 0.2 s at most |
| `SpawnSystem` | — | Spawns a Bydo beyond the right edge every 0.6–1.8 s while a game is running |
| `EnemyMovementSystem` | Enemy, Position, Velocity, Hitbox | Straight, sine-wave or zigzag velocity |
| `InvulnerabilitySystem` | Invulnerable | Counts down, then removes the component |
| `MovementSystem` | Position, Velocity | `position += velocity × dt` |
| `BoundarySystem` | Position, Hitbox, PlayerInput | Keeps ships inside the 1920 × 1080 world |
| `CollisionSystem` | Position, Hitbox | Detects overlapping pairs with `engine_physics` and reports them to `Room` |
| `OffscreenSystem` | Projectile or Enemy | Removes missiles leaving the world and Bydos past the left edge |

Systems that need services from `Room` receive them as callbacks: `nextNetworkId` for the shooting
and spawn systems, `canSpawn` for spawning, and the collision listener.

## Game rules

| Rule | Value (`GameConstants.hpp`) |
|---|---|
| Ship speed | 600 units/s |
| Missile speed / fire interval / damage | 1500 units/s / 0.2 s / 1 |
| Bydo speed / health / score | 250 units/s / 1 / 100 points |
| Lives | 3 |
| Respawn delay / invulnerability | 2 s / 2 s |
| Game over → new round | 5 s |

Collision outcomes are decided in `Room::onCollision`:

- missile × Bydo: the missile disappears, the Bydo loses health; if destroyed, the shooter scores;
- ship × Bydo: unless the ship is invulnerable, both are destroyed and the player loses a life.

The game state goes from *waiting* (no player) to *playing* (a player joined) to *game over* (no
present player has lives left), then back to *playing* after 5 s with full lives and an empty world.

## Snapshots and events

Destroyed entities and killed players are queued in `GameEvents` and broadcast before the next
snapshot. Every third tick, `collectNetworkEntities` lists every entity with `Position` and
`Replicated`, player ships first, capped at 150 entities. Shooting and spawning are refused when this
cap is reached, so a snapshot is always complete.
