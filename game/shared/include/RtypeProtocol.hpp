#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace rtype::protocol {

inline constexpr std::uint16_t MAGIC = 0x5254;
inline constexpr std::uint8_t VERSION = 1;
inline constexpr std::size_t HEADER_SIZE = 10;
inline constexpr std::size_t MAX_PACKET_BYTES = 1200;
inline constexpr std::size_t MAX_ENTITIES = 150;
inline constexpr std::size_t MAX_PLAYERS = 4;
inline constexpr std::uint16_t DEFAULT_PORT = 4242;

inline constexpr std::uint8_t BUTTON_UP = 0x01;
inline constexpr std::uint8_t BUTTON_DOWN = 0x02;
inline constexpr std::uint8_t BUTTON_LEFT = 0x04;
inline constexpr std::uint8_t BUTTON_RIGHT = 0x08;
inline constexpr std::uint8_t BUTTON_FIRE = 0x10;
inline constexpr std::uint8_t RESERVED_BUTTONS = 0xE0;

inline constexpr std::uint8_t KIND_PLAYER_SHIP = 1;

inline constexpr std::uint8_t REJECT_SERVER_FULL = 1;
inline constexpr std::uint8_t REJECT_VERSION_MISMATCH = 2;
inline constexpr std::uint8_t LEFT_QUIT = 1;
inline constexpr std::uint8_t LEFT_TIMEOUT = 2;

inline constexpr std::uint8_t SLOT_EMPTY = 0;
inline constexpr std::uint8_t SLOT_ALIVE = 1;
inline constexpr std::uint8_t SLOT_DEAD = 2;

inline constexpr std::uint8_t GAME_WAITING = 0;
inline constexpr std::uint8_t GAME_PLAYING = 1;
inline constexpr std::uint8_t GAME_OVER = 2;

inline constexpr int TICK_RATE = 60;
inline constexpr std::uint32_t SNAPSHOT_INTERVAL_TICKS = 3;
inline constexpr std::chrono::microseconds INPUT_INTERVAL{33333};
inline constexpr std::chrono::milliseconds INPUT_RELEASE_DELAY{500};
inline constexpr std::chrono::seconds HEARTBEAT_INTERVAL{1};
inline constexpr std::chrono::seconds CONNECT_RETRY_INTERVAL{1};
inline constexpr unsigned CONNECT_ATTEMPTS = 5;
inline constexpr std::chrono::seconds SESSION_TIMEOUT{5};

enum class MessageType : std::uint8_t
{
    Connect = 0x01,
    Input = 0x02,
    Heartbeat = 0x03,
    Disconnect = 0x04,
    Accept = 0x81,
    Reject = 0x82,
    Snapshot = 0x83,
    HeartbeatAck = 0x84,
    EntityDestroyed = 0x85,
    PlayerJoined = 0x86,
    PlayerKilled = 0x87,
    PlayerLeft = 0x88,
    ServerShutdown = 0x89
};

enum class Receiver
{
    Server,
    Client
};

struct PlayerState
{
    std::uint8_t state = SLOT_EMPTY;
    std::uint8_t lives = 0;
    std::uint32_t score = 0;
    std::uint16_t entityId = 0;
};

struct NetworkEntity
{
    std::uint16_t id = 0;
    std::uint8_t kind = KIND_PLAYER_SHIP;
    std::int16_t x = 0;
    std::int16_t y = 0;
};

struct WorldSnapshot
{
    std::uint32_t tick = 0;
    std::uint8_t gameState = GAME_WAITING;
    std::array<PlayerState, MAX_PLAYERS> players{};
    std::vector<NetworkEntity> entities;
};

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
