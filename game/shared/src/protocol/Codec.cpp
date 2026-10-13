#include "RtypeProtocol.hpp"

#include <unordered_set>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "GameError.hpp"
#include "NetError.hpp"

namespace rtype::protocol {

namespace {

constexpr std::size_t PAYLOAD_SIZE_OFFSET = 8;
constexpr std::size_t ENTITY_SIZE = 7;
constexpr std::uint8_t TO_CLIENT_BIT = 0x80;

bool validPlayerId(std::uint8_t playerId)
{
    return playerId < MAX_PLAYERS;
}

bool validRejectReason(std::uint8_t reason)
{
    return reason == REJECT_SERVER_FULL || reason == REJECT_VERSION_MISMATCH;
}

bool validLeftReason(std::uint8_t reason)
{
    return reason == LEFT_QUIT || reason == LEFT_TIMEOUT;
}

bool isPlayerShip(const WorldSnapshot &snapshot, std::uint16_t id)
{
    for (const auto &entity : snapshot.entities) {
        if (entity.id == id)
            return entity.kind == KIND_PLAYER_SHIP;
    }
    return false;
}

bool validPlayer(const PlayerState &player, const WorldSnapshot &snapshot, std::unordered_set<std::uint16_t> &ships)
{
    if (player.state == SLOT_EMPTY)
        return player.lives == 0 && player.score == 0 && player.entityId == 0;
    if (player.state == SLOT_DEAD)
        return player.entityId == 0;
    if (player.state != SLOT_ALIVE)
        return false;
    return player.lives != 0 && ships.insert(player.entityId).second && isPlayerShip(snapshot, player.entityId);
}

bool validSnapshot(const WorldSnapshot &snapshot)
{
    if (snapshot.gameState > GAME_OVER || snapshot.entities.size() > MAX_ENTITIES)
        return false;
    std::unordered_set<std::uint16_t> ids;
    for (const auto &entity : snapshot.entities) {
        if (entity.id == 0 || !ids.insert(entity.id).second)
            return false;
    }
    std::unordered_set<std::uint16_t> ships;
    for (const auto &player : snapshot.players) {
        if (!validPlayer(player, snapshot, ships))
            return false;
    }
    return true;
}

bool valid(const Packet &packet)
{
    switch (packet.type) {
    case MessageType::Connect:
    case MessageType::ServerShutdown:
    case MessageType::HeartbeatAck:
        return true;
    case MessageType::Input:
        return packet.token != 0 && (packet.buttons & RESERVED_BUTTONS) == 0;
    case MessageType::Heartbeat:
    case MessageType::Disconnect:
        return packet.token != 0;
    case MessageType::Accept:
        return packet.token != 0 && validPlayerId(packet.playerId);
    case MessageType::Reject:
        return validRejectReason(packet.reason);
    case MessageType::EntityDestroyed:
        return packet.entityId != 0;
    case MessageType::PlayerJoined:
    case MessageType::PlayerKilled:
        return validPlayerId(packet.playerId);
    case MessageType::PlayerLeft:
        return validPlayerId(packet.playerId) && validLeftReason(packet.reason);
    case MessageType::Snapshot:
        return validSnapshot(packet.snapshot);
    }
    return false;
}

void writeSnapshot(engine::net::ByteWriter &writer, const WorldSnapshot &snapshot)
{
    writer.writeU32(snapshot.tick);
    writer.writeU8(snapshot.gameState);
    for (const auto &player : snapshot.players) {
        writer.writeU8(player.state);
        writer.writeU8(player.lives);
        writer.writeU32(player.score);
        writer.writeU16(player.entityId);
    }
    writer.writeU8(static_cast<std::uint8_t>(snapshot.entities.size()));
    for (const auto &entity : snapshot.entities) {
        writer.writeU16(entity.id);
        writer.writeU8(entity.kind);
        writer.writeI16(entity.x);
        writer.writeI16(entity.y);
    }
}

void writePayload(engine::net::ByteWriter &writer, const Packet &packet)
{
    switch (packet.type) {
    case MessageType::Connect:
    case MessageType::ServerShutdown:
        break;
    case MessageType::Input:
        writer.writeU32(packet.token);
        writer.writeU8(packet.buttons);
        break;
    case MessageType::Heartbeat:
        writer.writeU32(packet.token);
        writer.writeU32(packet.timestamp);
        break;
    case MessageType::Disconnect:
        writer.writeU32(packet.token);
        break;
    case MessageType::Accept:
        writer.writeU32(packet.token);
        writer.writeU8(packet.playerId);
        break;
    case MessageType::Reject:
        writer.writeU8(packet.reason);
        break;
    case MessageType::HeartbeatAck:
        writer.writeU32(packet.timestamp);
        break;
    case MessageType::EntityDestroyed:
        writer.writeU16(packet.entityId);
        break;
    case MessageType::PlayerJoined:
    case MessageType::PlayerKilled:
        writer.writeU8(packet.playerId);
        break;
    case MessageType::PlayerLeft:
        writer.writeU8(packet.playerId);
        writer.writeU8(packet.reason);
        break;
    case MessageType::Snapshot:
        writeSnapshot(writer, packet.snapshot);
        break;
    }
}

bool readSnapshot(engine::net::ByteReader &reader, WorldSnapshot &snapshot)
{
    snapshot.tick = reader.readU32();
    snapshot.gameState = reader.readU8();
    for (auto &player : snapshot.players) {
        player.state = reader.readU8();
        player.lives = reader.readU8();
        player.score = reader.readU32();
        player.entityId = reader.readU16();
    }
    const std::size_t count = reader.readU8();
    if (count > MAX_ENTITIES || reader.remaining() != count * ENTITY_SIZE)
        return false;
    snapshot.entities.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        NetworkEntity entity;
        entity.id = reader.readU16();
        entity.kind = reader.readU8();
        entity.x = reader.readI16();
        entity.y = reader.readI16();
        snapshot.entities.push_back(entity);
    }
    return true;
}

bool readPayload(engine::net::ByteReader &reader, Packet &packet)
{
    switch (packet.type) {
    case MessageType::Connect:
    case MessageType::ServerShutdown:
        return true;
    case MessageType::Input:
        packet.token = reader.readU32();
        packet.buttons = reader.readU8();
        return true;
    case MessageType::Heartbeat:
        packet.token = reader.readU32();
        packet.timestamp = reader.readU32();
        return true;
    case MessageType::Disconnect:
        packet.token = reader.readU32();
        return true;
    case MessageType::Accept:
        packet.token = reader.readU32();
        packet.playerId = reader.readU8();
        return true;
    case MessageType::Reject:
        packet.reason = reader.readU8();
        return true;
    case MessageType::HeartbeatAck:
        packet.timestamp = reader.readU32();
        return true;
    case MessageType::EntityDestroyed:
        packet.entityId = reader.readU16();
        return true;
    case MessageType::PlayerJoined:
    case MessageType::PlayerKilled:
        packet.playerId = reader.readU8();
        return true;
    case MessageType::PlayerLeft:
        packet.playerId = reader.readU8();
        packet.reason = reader.readU8();
        return true;
    case MessageType::Snapshot:
        return readSnapshot(reader, packet.snapshot);
    }
    return false;
}

} // namespace

bool isNewer(std::uint32_t candidate, std::uint32_t previous)
{
    const std::uint32_t distance = candidate - previous;
    return distance != 0 && distance < 0x80000000u;
}

std::vector<std::uint8_t> encode(const Packet &packet)
{
    if (!valid(packet))
        throw GameError("Codec: invalid outgoing packet");
    engine::net::ByteWriter writer(MAX_PACKET_BYTES);
    writer.writeU16(MAGIC);
    writer.writeU8(packet.version);
    writer.writeU8(static_cast<std::uint8_t>(packet.type));
    writer.writeU32(packet.sequence);
    writer.writeU16(0);
    writePayload(writer, packet);
    writer.patchU16(PAYLOAD_SIZE_OFFSET, static_cast<std::uint16_t>(writer.size() - HEADER_SIZE));
    return writer.release();
}

std::optional<Packet> decode(std::span<const std::uint8_t> bytes, Receiver receiver)
{
    if (bytes.size() < HEADER_SIZE || bytes.size() > MAX_PACKET_BYTES)
        return std::nullopt;
    try {
        engine::net::ByteReader reader(bytes);
        if (reader.readU16() != MAGIC)
            return std::nullopt;
        Packet packet;
        packet.version = reader.readU8();
        packet.type = static_cast<MessageType>(reader.readU8());
        const bool toClient = (static_cast<std::uint8_t>(packet.type) & TO_CLIENT_BIT) != 0;
        if (toClient != (receiver == Receiver::Client))
            return std::nullopt;
        packet.sequence = reader.readU32();
        if (reader.readU16() != bytes.size() - HEADER_SIZE)
            return std::nullopt;
        if (packet.version != VERSION && packet.type != MessageType::Connect && packet.type != MessageType::Reject)
            return std::nullopt;
        if (!readPayload(reader, packet) || reader.remaining() != 0 || !valid(packet))
            return std::nullopt;
        return packet;
    } catch (const engine::net::NetError &) {
        return std::nullopt;
    }
}

} // namespace rtype::protocol
