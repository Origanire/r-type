# Onboarding

This page is for a developer joining the team. It gives the mental model of the project in one
read, then points to the details.

## The project in five sentences

1. Two programs: `r-type_server` runs the game, `r-type_client` displays it and sends the keys.
2. The server is the authority: it simulates everything at 60 ticks per second and sends a complete
   snapshot of the world 20 times per second over UDP.
3. The code is split into a reusable **engine** (`engine/`) and the **game** (`game/`); the engine
   never includes game code.
4. Game objects are **entities** made of **components** (plain structs) and updated by **systems**
   (classes implementing `ISystem`).
5. The engine records which components each entity has (a bitmask); the game stores the values in
   one array per component, inside `ServerWorld` or `ClientWorld`.

## First hour

1. Build and run the game: [Installation](../getting-started/installation.md),
   [Building](../getting-started/building.md), [Running](../getting-started/running.md).
2. Read the [architecture overview](../architecture/overview.md) and the [ECS](../architecture/ecs.md).
3. Read these files, in this order:

| File | What you learn |
|---|---|
| `game/server/src/main.cpp` | The fixed-step server loop |
| `game/server/include/ServerWorld.hpp` | Every server component and its bit |
| `game/server/src/EntityFactory.cpp` | How an entity is built |
| `game/server/src/Room.cpp` | System order and game rules |
| `game/server/src/systems/MovementSystem.cpp` | The shape of every system |
| `game/client/src/Client.cpp` | The client loop and snapshot application |
| `game/shared/include/RtypeProtocol.hpp` | Every message and constant of the protocol |

## Where does my change go?

| Change | Place |
|---|---|
| A gameplay rule (damage, score, spawning) | A server system or `Room` |
| What something looks like | Client: `SpriteSheet.hpp`, `Client.cpp`, client systems |
| A tuning value | `game/shared/include/GameConstants.hpp` |
| A new message or field on the wire | `RtypeProtocol.hpp`, `Codec.cpp` and the [protocol](../protocol.md), together |
| Something every game could use | An engine module, with no reference to R-Type |

## Rules that are easy to miss

- No function body in a header, no comments in code, one exception class per module (see
  [conventions](conventions.md)).
- A new `.cpp` must be added to `CMakeLists.txt`.
- When creating an entity, write every component value before setting its bits.
- Never let the client decide a gameplay outcome.

## Glossary

| Term | Meaning |
|---|---|
| Tick | One server simulation step (1/60 s) |
| Snapshot | Complete list of replicated entities sent to clients |
| Slot | Player number, 0 to 3 |
| Network id | Entity id shared by server and clients (different from the ECS index) |
| Kind | Entity type on the wire (1 ship, 2 player missile, 3 Bydo) |
| Generation | Counter making old entity handles invalid after reuse |
