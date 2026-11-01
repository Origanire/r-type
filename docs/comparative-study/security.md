# Security

## Vulnerabilities per technology

| Technology | Main risks |
|---|---|
| UDP | Source address spoofing, flooding, amplification (small request, large reply), loss and reordering exploited to confuse state |
| C++ | Buffer overflows, out-of-bounds reads, use-after-free, uncaught exceptions terminating the process |
| Binary protocol | Malformed lengths, oversized counts, invalid enum values, forged fields |
| Third-party libraries (SFML, Asio) | Vulnerabilities in image decoding or networking code |
| Asset files | Malformed images loaded by the client |

## Mitigations implemented

| Risk | Mitigation | Where |
|---|---|---|
| Out-of-bounds reads | Every read is bounds-checked; a short packet throws and is dropped | `ByteReader` |
| Oversized packets | Datagrams over 1200 bytes dropped; writer refuses to exceed its limit | `UdpTransport`, `ByteWriter` |
| Malformed packets | Magic, version, direction, payload size, exact length, field ranges, entity count and consistency checked; any failure ignores the packet | `Codec.cpp` |
| Struct layout attacks | No struct is ever copied to or from the wire | Codec |
| Memory exhaustion | Receive queue capped at 256 datagrams, pending sends at 256, snapshots at 150 entities, 4 sessions | `UdpTransport`, `Server` |
| Connection flooding | At most 32 CONNECT handled per second; ACCEPT repeated at most every 200 ms per endpoint | `Server` |
| Spoofed input | A packet must come from the session's endpoint and carry its random non-zero token | `Server` |
| Replay and reordering | Sequence numbers; older or duplicate packets ignored | `Server`, `NetworkClient` |
| Amplification | No snapshot is sent before the endpoint confirms its token | `Server` |
| Cheating | All gameplay decided on the server | Architecture |
| Crashes | All errors are project exceptions caught in `main`; network thread exceptions are forwarded to the main thread | Entry points, `UdpTransport` |
| Memory safety | RAII, containers, `std::span`, no `new`/`delete` | Whole code base |
| Dependency drift | vcpkg baseline pinned: everyone builds the same reviewed versions | `vcpkg.json` |

## Known limits

- Tokens prove endpoint ownership, not identity: there is no encryption, signature or account.
  An attacker who can observe traffic can impersonate a player.
- A modified packet whose fields remain valid is accepted (no checksum or MAC).
- Silent drops are not counted yet.

## Monitoring

| Today | Planned |
|---|---|
| Server console logs joins, departures (with reason), kills and game state changes | Counters for dropped packets per reason and per endpoint, printed periodically |
| Network thread failures are reported and stop the program with code 84 | Rate alerts when an endpoint exceeds normal traffic |
| | Regular `vcpkg` baseline updates and review of security advisories for SFML and Asio |
