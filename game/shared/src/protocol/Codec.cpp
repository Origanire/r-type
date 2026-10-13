#include "RtypeProtocol.hpp"

#include <bit>
#include <stdexcept>
#include <unordered_set>

namespace rtype::protocol {
namespace {

class Reader
{
public:
    explicit Reader(std::span<const std::uint8_t> bytes) : m_bytes(bytes) {}
    std::uint32_t read(unsigned count)
    {
        if (count > m_bytes.size() - m_offset)
            throw std::runtime_error("Truncated packet");
        std::uint32_t value = 0;
        for (unsigned i = 0; i < count; ++i)
            value = (value << 8) | m_bytes[m_offset++];
        return value;
    }
    std::int16_t signed16() { return std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(read(2))); }
    bool done() const { return m_offset == m_bytes.size(); }
private:
    std::span<const std::uint8_t> m_bytes;
    std::size_t m_offset = 0;
};

class Writer
{
public:
    std::vector<std::uint8_t> bytes;
    void write(std::uint32_t value, unsigned count)
    {
        if (bytes.size() + count > MAX_PACKET_BYTES)
            throw std::runtime_error("Packet too large");
        for (unsigned i = count; i > 0; --i)
            bytes.push_back(static_cast<std::uint8_t>(value >> ((i - 1) * 8)));
    }
};

bool valid(const Packet &p)
{
    switch (p.type) {
    case MessageType::Connect: case MessageType::ServerShutdown: case MessageType::HeartbeatAck: return true;
    case MessageType::Input: return p.token != 0 && (p.buttons & 0xE0) == 0;
    case MessageType::Heartbeat: case MessageType::Disconnect: return p.token != 0;
    case MessageType::Accept: return p.token != 0 && p.playerId < 4;
    case MessageType::Reject: return p.reason == 1 || p.reason == 2;
    case MessageType::EntityDestroyed: return p.entityId != 0;
    case MessageType::PlayerJoined: case MessageType::PlayerKilled: return p.playerId < 4;
    case MessageType::PlayerLeft: return p.playerId < 4 && (p.reason == 1 || p.reason == 2);
    case MessageType::Snapshot: {
        if (p.snapshot.gameState > 2 || p.snapshot.entities.size() > MAX_ENTITIES)
            return false;
        std::unordered_set<std::uint16_t> ids;
        for (const auto &entity : p.snapshot.entities)
            if (entity.id == 0 || !ids.insert(entity.id).second)
                return false;
        std::unordered_set<std::uint16_t> ships;
        for (const auto &player : p.snapshot.players) {
            if (player.state > 2)
                return false;
            if (player.state == 0 && (player.lives != 0 || player.score != 0 || player.entityId != 0))
                return false;
            if (player.state == 1) {
                if (!ships.insert(player.entityId).second || player.lives == 0 || !ids.contains(player.entityId))
                    return false;
                for (const auto &entity : p.snapshot.entities)
                    if (entity.id == player.entityId && entity.kind != 1)
                        return false;
            } else if (player.entityId != 0)
                return false;
        }
        return true;
    }
    }
    return false;
}

} // namespace

bool isNewer(std::uint32_t candidate, std::uint32_t previous)
{
    const auto distance = candidate - previous;
    return distance != 0 && distance < 0x80000000u;
}

std::vector<std::uint8_t> encode(const Packet &p)
{
    if (!valid(p))
        throw std::runtime_error("Invalid outgoing packet");
    Writer w;
    w.write(MAGIC, 2); w.write(p.version, 1); w.write(static_cast<std::uint8_t>(p.type), 1);
    w.write(p.sequence, 4); w.write(0, 2);
    switch (p.type) {
    case MessageType::Connect: case MessageType::ServerShutdown: break;
    case MessageType::Input: w.write(p.token, 4); w.write(p.buttons, 1); break;
    case MessageType::Heartbeat: w.write(p.token, 4); w.write(p.timestamp, 4); break;
    case MessageType::Disconnect: w.write(p.token, 4); break;
    case MessageType::Accept: w.write(p.token, 4); w.write(p.playerId, 1); break;
    case MessageType::Reject: w.write(p.reason, 1); break;
    case MessageType::HeartbeatAck: w.write(p.timestamp, 4); break;
    case MessageType::EntityDestroyed: w.write(p.entityId, 2); break;
    case MessageType::PlayerJoined: case MessageType::PlayerKilled: w.write(p.playerId, 1); break;
    case MessageType::PlayerLeft: w.write(p.playerId, 1); w.write(p.reason, 1); break;
    case MessageType::Snapshot:
        w.write(p.snapshot.tick, 4); w.write(p.snapshot.gameState, 1);
        for (const auto &player : p.snapshot.players) {
            w.write(player.state, 1); w.write(player.lives, 1); w.write(player.score, 4); w.write(player.entityId, 2);
        }
        w.write(static_cast<std::uint32_t>(p.snapshot.entities.size()), 1);
        for (const auto &entity : p.snapshot.entities) {
            w.write(entity.id, 2); w.write(entity.kind, 1);
            w.write(static_cast<std::uint16_t>(entity.x), 2); w.write(static_cast<std::uint16_t>(entity.y), 2);
        }
        break;
    }
    const auto size = w.bytes.size() - 10;
    w.bytes[8] = static_cast<std::uint8_t>(size >> 8);
    w.bytes[9] = static_cast<std::uint8_t>(size);
    return w.bytes;
}

std::optional<Packet> decode(std::span<const std::uint8_t> bytes, Receiver receiver)
{
    if (bytes.size() < 10 || bytes.size() > MAX_PACKET_BYTES)
        return std::nullopt;
    try {
        Reader r(bytes);
        if (r.read(2) != MAGIC)
            return std::nullopt;
        Packet p;
        p.version = static_cast<std::uint8_t>(r.read(1));
        p.type = static_cast<MessageType>(r.read(1));
        const bool toClient = (static_cast<std::uint8_t>(p.type) & 0x80) != 0;
        if (toClient != (receiver == Receiver::Client))
            return std::nullopt;
        p.sequence = r.read(4);
        if (r.read(2) != bytes.size() - 10)
            return std::nullopt;
        if (p.version != VERSION && p.type != MessageType::Connect && p.type != MessageType::Reject)
            return std::nullopt;
        switch (p.type) {
        case MessageType::Connect: case MessageType::ServerShutdown: break;
        case MessageType::Input: p.token = r.read(4); p.buttons = static_cast<std::uint8_t>(r.read(1)); break;
        case MessageType::Heartbeat: p.token = r.read(4); p.timestamp = r.read(4); break;
        case MessageType::Disconnect: p.token = r.read(4); break;
        case MessageType::Accept: p.token = r.read(4); p.playerId = static_cast<std::uint8_t>(r.read(1)); break;
        case MessageType::Reject: p.reason = static_cast<std::uint8_t>(r.read(1)); break;
        case MessageType::HeartbeatAck: p.timestamp = r.read(4); break;
        case MessageType::EntityDestroyed: p.entityId = static_cast<std::uint16_t>(r.read(2)); break;
        case MessageType::PlayerJoined: case MessageType::PlayerKilled: p.playerId = static_cast<std::uint8_t>(r.read(1)); break;
        case MessageType::PlayerLeft: p.playerId = static_cast<std::uint8_t>(r.read(1)); p.reason = static_cast<std::uint8_t>(r.read(1)); break;
        case MessageType::Snapshot: {
            p.snapshot.tick = r.read(4); p.snapshot.gameState = static_cast<std::uint8_t>(r.read(1));
            for (auto &player : p.snapshot.players) {
                player.state = static_cast<std::uint8_t>(r.read(1)); player.lives = static_cast<std::uint8_t>(r.read(1));
                player.score = r.read(4); player.entityId = static_cast<std::uint16_t>(r.read(2));
            }
            const auto count = r.read(1);
            if (count > MAX_ENTITIES || bytes.size() != 48 + count * 7)
                return std::nullopt;
            for (std::uint32_t i = 0; i < count; ++i) {
                NetworkEntity entity;
                entity.id = static_cast<std::uint16_t>(r.read(2)); entity.kind = static_cast<std::uint8_t>(r.read(1));
                entity.x = r.signed16(); entity.y = r.signed16();
                p.snapshot.entities.push_back(entity);
            }
            break;
        }
        default: return std::nullopt;
        }
        return r.done() && valid(p) ? std::optional<Packet>(std::move(p)) : std::nullopt;
    } catch (const std::runtime_error &) {
        return std::nullopt;
    }
}

} // namespace rtype::protocol
