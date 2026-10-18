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

**Choice:** complete snapshots at 20 Hz with a 60 Hz simulation, at most 1098 bytes each. Events
(destroyed, killed, joined, left) are sent in addition, for presentation only.

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
