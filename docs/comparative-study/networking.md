# Networking

## Transport

| Option | Strengths | Weaknesses |
|---|---|---|
| **UDP** | No head-of-line blocking, low latency, datagram boundaries preserved | No delivery, ordering or duplicate protection |
| TCP | Reliable, ordered | One lost packet delays everything behind it; unsuitable for real-time state |
| UDP + TCP | Reliable side channel | Two connections to manage; justification required by the subject |

**Choice:** UDP only, as required for in-game traffic. The protocol tolerates loss by design.

## Encoding

| Option | Strengths | Weaknesses |
|---|---|---|
| Text (JSON, lines) | Human-readable | Larger, parsing cost — forbidden by the subject |
| Raw struct copy | Fast to write | Padding and byte order depend on compiler and CPU |
| **Explicit binary, big-endian, field by field** | Compact, identical on every platform, bounds-checked | Codec code to write |

## Authority model

| Model | Consistency | Cheating | Responsiveness |
|---|---|---|---|
| **Server authoritative, client displays** (ours) | One truth | Prevented | One round trip for own movement |
| Server authoritative + client prediction | One truth | Prevented | Immediate, needs reconciliation |
| Peer-to-peer lockstep | All peers equal | Hard to prevent | Slowest peer blocks everyone |
| Host client | Host is the truth | Host can cheat | Host has no latency |

**Choice:** server authoritative, as required. Prediction is the planned next step.

## Synchronization

| Option | Loss tolerance | Bandwidth |
|---|---|---|
| Events only ("spawned", "moved") | A lost event desynchronizes forever | Low |
| **Complete snapshots** | Each snapshot is self-sufficient | Higher |
| Delta snapshots against acknowledged state | Tolerant with acknowledgements | Lowest, more complex |

**Choice:** complete snapshots at 20 Hz with a 60 Hz simulation, at most 1134 bytes each for R-Type (the size is
checked at start-up from the game file). Events (entity destroyed, player killed, sounds) are sent in addition,
for presentation only: losing one never breaks the state.

## Input

| Option | Effect of a lost packet |
|---|---|
| Key events ("pressed", "released") | A lost "released" leaves a key stuck |
| **Held-button state, repeated ~30/s** | Corrected by the next packet; released after 500 ms without input |

## Rates

| Value | Reason |
|---|---|
| 60 ticks/s | Smooth simulation, matches display rate |
| 20 snapshots/s | One every 3 ticks; balance between smoothness and bandwidth |
| ~30 inputs/s | Below one frame of delay at 60 Hz |
| 1 heartbeat/s, 5 s timeout | Detects crashed peers quickly without traffic overhead |

## Solo transport

The same snapshots can be produced and consumed in one process. The solo mode calls the simulation directly
instead of sending datagrams, which proves the separation between the game state and its transport: no
encoding, no socket, the same simulation and the same display code.

## Lobby

| Option | Strengths | Weaknesses |
|---|---|---|
| One game per server process | Simple | One port and one process per group of players |
| Separate lobby server | Scales to many servers | A second program and protocol |
| **Rooms inside one server** (ours) | One process, one port, up to 16 rooms, each with its own simulation and rules | All rooms share one machine |

Rooms are messages of the same protocol (`CREATE_ROOM`, `JOIN_ROOM`, `ROOM_UPDATE`...), so a single UDP socket
serves the lobby and every game.
