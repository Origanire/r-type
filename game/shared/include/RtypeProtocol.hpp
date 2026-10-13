#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace rtype::protocol {

inline constexpr std::uint16_t MAGIC = 0x5254;
inline constexpr std::uint8_t VERSION = 1;
inline constexpr std::size_t MAX_ENTITIES = 150;
inline constexpr std::size_t MAX_PACKET_BYTES = 1200;
inline constexpr std::uint16_t DEFAULT_PORT = 4242;

enum class MessageType : std::uint8_t
{
    Connect = 0x01, Input = 0x02, Heartbeat = 0x03, Disconnect = 0x04,
    Accept = 0x81, Reject = 0x82, Snapshot = 0x83, HeartbeatAck = 0x84,
    EntityDestroyed = 0x85, PlayerJoined = 0x86, PlayerKilled = 0x87,
    PlayerLeft = 0x88, ServerShutdown = 0x89
};

enum class Receiver { Server, Client };

struct PlayerState
{
    std::uint8_t state = 0;
    std::uint8_t lives = 0;
    std::uint32_t score = 0;
    std::uint16_t entityId = 0;
};

struct NetworkEntity
{
    std::uint16_t id = 0;
    std::uint8_t kind = 1;
    std::int16_t x = 0;
    std::int16_t y = 0;
};

struct WorldSnapshot
{
    std::uint32_t tick = 0;
    std::uint8_t gameState = 0;
    std::array<PlayerState, 4> players{};
    std::vector<NetworkEntity> entities;
};

// Only the fields belonging to type are serialized; no struct is copied to the wire.
struct Packet
{
    MessageType type = MessageType::Connect;
    std::uint8_t version = VERSION;
    std::uint32_t sequence = 0;
    std::uint32_t token = 0;
    std::uint32_t timestamp = 0;
    std::uint8_t buttons = 0;
    std::uint8_t playerId = 0;
    std::uint8_t reason = 0;
    std::uint16_t entityId = 0;
    WorldSnapshot snapshot;
};

bool isNewer(std::uint32_t candidate, std::uint32_t previous);
std::vector<std::uint8_t> encode(const Packet &packet);
std::optional<Packet> decode(std::span<const std::uint8_t> bytes, Receiver receiver);

} // namespace rtype::protocol
