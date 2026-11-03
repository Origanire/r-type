#include "Codec.hpp"

#include <utility>

#include "NetError.hpp"

namespace engine::network {

Codec::Codec(NetworkSettings settings) : m_settings(std::move(settings))
{
}

bool Codec::isNewer(std::uint32_t candidate, std::uint32_t previous)
{
    const std::uint32_t distance = candidate - previous;
    return distance != 0 && distance < HALF_SEQUENCE_RANGE;
}

std::vector<std::uint8_t> Codec::encode(const Packet &packet) const
{
    if (!isValid(packet))
        throw net::NetError("Codec: invalid outgoing packet");
    net::ByteWriter writer(Protocol::MAX_PACKET_BYTES);
    writer.writeU16(Protocol::MAGIC);
    writer.writeU8(packet.version());
    writer.writeU8(static_cast<std::uint8_t>(packet.type()));
    writer.writeU32(packet.sequence());
    writer.writeU16(0);
    writeBody(writer, packet);
    writer.patchU16(PAYLOAD_SIZE_OFFSET, static_cast<std::uint16_t>(writer.size() - Protocol::HEADER_SIZE));
    return writer.release();
}

std::optional<Packet> Codec::decode(std::span<const std::uint8_t> bytes, Receiver receiver) const
{
    if (bytes.size() < Protocol::HEADER_SIZE || bytes.size() > Protocol::MAX_PACKET_BYTES)
        return std::nullopt;
    try {
        net::ByteReader reader(bytes);
        if (reader.readU16() != Protocol::MAGIC)
            return std::nullopt;
        const std::uint8_t version = reader.readU8();
        const std::uint8_t rawType = reader.readU8();
        const bool toClient = (rawType & TO_CLIENT_BIT) != 0;
        if (toClient != (receiver == Receiver::Client))
            return std::nullopt;
        const auto type = static_cast<MessageType>(rawType);
        const std::uint32_t sequence = reader.readU32();
        if (reader.readU16() != bytes.size() - Protocol::HEADER_SIZE)
            return std::nullopt;
        if (version != Protocol::VERSION && type != MessageType::Connect && type != MessageType::Reject)
            return std::nullopt;
        Packet packet = readBody(reader, type, sequence, version);
        if (reader.remaining() != 0 || !isValid(packet))
            return std::nullopt;
        return packet;
    } catch (const net::NetError &) {
        return std::nullopt;
    }
}

bool Codec::isValid(const Packet &packet) const
{
    switch (packet.type()) {
    case MessageType::Connect:
        return packet.as<ConnectMessage>().isValid(m_settings);
    case MessageType::Input:
        return packet.as<InputMessage>().isValid(m_settings);
    case MessageType::Heartbeat:
        return packet.as<HeartbeatMessage>().isValid(m_settings);
    case MessageType::Disconnect:
        return packet.as<DisconnectMessage>().isValid(m_settings);
    case MessageType::Accept:
        return packet.as<AcceptMessage>().isValid(m_settings);
    case MessageType::Reject:
        return packet.as<RejectMessage>().isValid(m_settings);
    case MessageType::Snapshot:
        return packet.as<SnapshotMessage>().isValid(m_settings);
    case MessageType::HeartbeatAck:
        return packet.as<HeartbeatAckMessage>().isValid(m_settings);
    case MessageType::EntityDestroyed:
        return packet.as<EntityDestroyedMessage>().isValid(m_settings);
    case MessageType::PlayerJoined:
        return packet.as<PlayerJoinedMessage>().isValid(m_settings);
    case MessageType::PlayerKilled:
        return packet.as<PlayerKilledMessage>().isValid(m_settings);
    case MessageType::PlayerLeft:
        return packet.as<PlayerLeftMessage>().isValid(m_settings);
    case MessageType::ServerShutdown:
        return packet.as<ServerShutdownMessage>().isValid(m_settings);
    }
    return false;
}

void Codec::writeBody(net::ByteWriter &writer, const Packet &packet)
{
    switch (packet.type()) {
    case MessageType::Connect:
        return packet.as<ConnectMessage>().write(writer);
    case MessageType::Input:
        return packet.as<InputMessage>().write(writer);
    case MessageType::Heartbeat:
        return packet.as<HeartbeatMessage>().write(writer);
    case MessageType::Disconnect:
        return packet.as<DisconnectMessage>().write(writer);
    case MessageType::Accept:
        return packet.as<AcceptMessage>().write(writer);
    case MessageType::Reject:
        return packet.as<RejectMessage>().write(writer);
    case MessageType::Snapshot:
        return packet.as<SnapshotMessage>().write(writer);
    case MessageType::HeartbeatAck:
        return packet.as<HeartbeatAckMessage>().write(writer);
    case MessageType::EntityDestroyed:
        return packet.as<EntityDestroyedMessage>().write(writer);
    case MessageType::PlayerJoined:
        return packet.as<PlayerJoinedMessage>().write(writer);
    case MessageType::PlayerKilled:
        return packet.as<PlayerKilledMessage>().write(writer);
    case MessageType::PlayerLeft:
        return packet.as<PlayerLeftMessage>().write(writer);
    case MessageType::ServerShutdown:
        return packet.as<ServerShutdownMessage>().write(writer);
    }
}

Packet Codec::readBody(
    net::ByteReader &reader, MessageType type, std::uint32_t sequence, std::uint8_t version)
{
    switch (type) {
    case MessageType::Connect:
        return readAs<ConnectMessage>(reader, sequence, version);
    case MessageType::Input:
        return readAs<InputMessage>(reader, sequence, version);
    case MessageType::Heartbeat:
        return readAs<HeartbeatMessage>(reader, sequence, version);
    case MessageType::Disconnect:
        return readAs<DisconnectMessage>(reader, sequence, version);
    case MessageType::Accept:
        return readAs<AcceptMessage>(reader, sequence, version);
    case MessageType::Reject:
        return readAs<RejectMessage>(reader, sequence, version);
    case MessageType::Snapshot:
        return readAs<SnapshotMessage>(reader, sequence, version);
    case MessageType::HeartbeatAck:
        return readAs<HeartbeatAckMessage>(reader, sequence, version);
    case MessageType::EntityDestroyed:
        return readAs<EntityDestroyedMessage>(reader, sequence, version);
    case MessageType::PlayerJoined:
        return readAs<PlayerJoinedMessage>(reader, sequence, version);
    case MessageType::PlayerKilled:
        return readAs<PlayerKilledMessage>(reader, sequence, version);
    case MessageType::PlayerLeft:
        return readAs<PlayerLeftMessage>(reader, sequence, version);
    case MessageType::ServerShutdown:
        return readAs<ServerShutdownMessage>(reader, sequence, version);
    }
    throw net::NetError("Codec: unknown message type");
}

} // namespace engine::network
