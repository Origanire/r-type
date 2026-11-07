# Networking

Networking is split in two engine modules:

| Module | Knows about | Main classes |
|---|---|---|
| `engine_net` | Bytes and datagrams only | `UdpTransport`, `Endpoint`, `Datagram`, `ByteWriter`, `ByteReader` |
| `engine_network` | The game protocol | `Packet`, `Codec`, the messages, `NetworkSettings`, `GameServer`, `ClientConnection` |

The wire format itself is specified in the [Network protocol (RFC)](../protocol.md).

## Transport

`UdpTransport` wraps an IPv4 UDP socket (Asio, hidden in `net/internal`). A background thread receives
datagrams and puts them in a queue; the main thread takes them with `poll()`.

| Limit | Value | When exceeded |
|---|---|---|
| Datagram size | 1200 bytes | Larger datagrams are dropped |
| Received queue | 256 datagrams | New datagrams are dropped until the main thread polls |
| Pending sends | 256 datagrams | New sends are dropped |

Dropping is the right reaction for a real-time game: a later snapshot or input replaces the lost one.

`UdpTransport::primaryLocalAddress()` finds the LAN address of the machine (it asks the system which interface
would route to a public address, without sending anything). The server prints it at start so players know what
to type after `-h`.

## Messages and packets

Each message is a class in `network/messages/<category>/` with three functions:

```cpp
bool isValid(const NetworkSettings &settings) const;   // field limits
void write(net::ByteWriter &writer) const;             // payload encoding
static XMessage read(net::ByteReader &reader);         // payload decoding
```

`Packet` holds the header fields and one message in a `std::variant`. `Packet::visit(visitor)` applies a
visitor to the message it holds. Three visitors (`network/protocol/include/PacketVisitors.hpp`) replace long
`switch` statements:

| Visitor | Used by | Job |
|---|---|---|
| `MessageValidator` | `Codec::isValid` | Calls `isValid` of the message |
| `MessageWriter` | `Codec::encode` | Calls `write` of the message |
| `TokenReader` | `GameServer` | Returns the session token of a client message, if it has one |

`Codec` turns a `Packet` into bytes and back, and drops anything malformed (see section 8 of the RFC).
Adding a message means: one class, one code in `MessageType`, one line in `Codec::readBody`, and one entry in the
variant of `Packet`. See [Add a network message](../guides/add-a-network-message.md).

## Settings

`NetworkSettings::fromGame` reads the `network` block of the game file, plus the sizes of `worldValues`,
`playerValues`, `rules` and `eventSounds`. Server and clients therefore agree on every list size without sending
it. At start, it checks that the largest possible snapshot fits in 1200 bytes and refuses to start otherwise.

## Client connection

`ClientConnection` (`network/client/`) is everything a client needs on the network side:

- sends `CONNECT` every `connectRetryIntervalMs` until `ACCEPT`, and gives up after `connectAttempts`;
- sends `HEARTBEAT` every `heartbeatIntervalMs` and measures the ping from `HEARTBEAT_ACK`;
- sends `INPUT` every `inputIntervalMs` while in game;
- keeps only the **newest** snapshot (older or reordered snapshots are dropped by tick number);
- queues every other event for the driver (`takeEvents()`);
- counts received and sent bytes for the debug overlay.

## Reliability strategy

| Data | Strategy |
|---|---|
| World state | Full snapshot 20 times per second: a lost snapshot is corrected by the next one |
| Inputs | Full button state 30 times per second: a lost input is corrected by the next one. Released after 500 ms of silence |
| Connection | `CONNECT` retried every second |
| Liveness | Heartbeats both ways, 5 s timeout |
| One-off events (death, sound) | Sent once. A lost one only loses a cosmetic effect, the next snapshot stays correct |
| Rooms | `ROOM_UPDATE` every second, the client leaves a room silent for 4 s |
