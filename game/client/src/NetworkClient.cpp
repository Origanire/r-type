#include "NetworkClient.hpp"

#include <iostream>
#include <utility>

#include "GameError.hpp"

namespace rtype::client {

using protocol::MessageType;

NetworkClient::NetworkClient(engine::net::Endpoint server) : m_server(std::move(server))
{
}

void NetworkClient::update(std::uint8_t buttons)
{
    const Time now = std::chrono::steady_clock::now();
    for (const auto &datagram : m_transport.poll()) {
        if (datagram.sender != m_server)
            continue;
        const auto packet = protocol::decode(datagram.bytes, protocol::Receiver::Client);
        if (packet && handlePacket(*packet))
            m_lastReceive = now;
    }
    if (!connected()) {
        retryConnect(now);
        return;
    }
    if (now - m_lastReceive >= protocol::SESSION_TIMEOUT)
        throw GameError("NetworkClient: connection lost, no server packet for five seconds");
    sendPeriodic(buttons, now);
}

void NetworkClient::disconnect()
{
    if (!connected())
        return;
    protocol::Packet packet;
    packet.type = MessageType::Disconnect;
    send(packet);
    m_token = 0;
}

std::vector<protocol::Packet> NetworkClient::takeEvents()
{
    std::vector<protocol::Packet> events;
    events.swap(m_events);
    return events;
}

std::optional<protocol::WorldSnapshot> NetworkClient::takeSnapshot()
{
    std::optional<protocol::WorldSnapshot> snapshot = std::move(m_snapshot);
    m_snapshot.reset();
    return snapshot;
}

bool NetworkClient::connected() const
{
    return m_token != 0;
}

bool NetworkClient::handlePacket(const protocol::Packet &packet)
{
    if (!connected())
        return handleConnectionReply(packet);
    if (packet.type == MessageType::Accept || packet.type == MessageType::Reject)
        return false;
    if (packet.type == MessageType::Snapshot) {
        if (m_lastTick && !protocol::isNewer(packet.snapshot.tick, *m_lastTick))
            return false;
        m_lastTick = packet.snapshot.tick;
        m_snapshot = packet.snapshot;
    } else if (m_serverSequence && !protocol::isNewer(packet.sequence, *m_serverSequence)) {
        return false;
    }
    if (packet.type == MessageType::ServerShutdown)
        throw GameError("NetworkClient: server stopped");
    if (packet.type == MessageType::EntityDestroyed || packet.type == MessageType::PlayerKilled)
        m_events.push_back(packet);
    if (!m_serverSequence || protocol::isNewer(packet.sequence, *m_serverSequence))
        m_serverSequence = packet.sequence;
    return true;
}

bool NetworkClient::handleConnectionReply(const protocol::Packet &packet)
{
    if (packet.type == MessageType::Reject) {
        if (packet.reason == protocol::REJECT_SERVER_FULL)
            throw GameError("NetworkClient: server full");
        throw GameError("NetworkClient: protocol version mismatch");
    }
    if (packet.type != MessageType::Accept || m_attempts == 0)
        return false;
    m_token = packet.token;
    m_serverSequence = packet.sequence;
    std::cout << "Connected as player " << static_cast<int>(packet.playerId) << std::endl;
    return true;
}

void NetworkClient::retryConnect(Time now)
{
    if (m_attempts != 0 && now - m_lastConnect < protocol::CONNECT_RETRY_INTERVAL)
        return;
    if (m_attempts >= protocol::CONNECT_ATTEMPTS)
        throw GameError("NetworkClient: connection timed out after five attempts");
    protocol::Packet packet;
    packet.type = MessageType::Connect;
    send(packet);
    ++m_attempts;
    m_lastConnect = now;
}

void NetworkClient::sendPeriodic(std::uint8_t buttons, Time now)
{
    if (now - m_lastInput >= protocol::INPUT_INTERVAL) {
        protocol::Packet input;
        input.type = MessageType::Input;
        input.buttons = buttons;
        send(input);
        m_lastInput = now;
    }
    if (now - m_lastHeartbeat >= protocol::HEARTBEAT_INTERVAL) {
        const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        protocol::Packet heartbeat;
        heartbeat.type = MessageType::Heartbeat;
        heartbeat.timestamp = static_cast<std::uint32_t>(milliseconds.count());
        send(heartbeat);
        m_lastHeartbeat = now;
    }
}

void NetworkClient::send(protocol::Packet packet)
{
    packet.sequence = m_sequence++;
    packet.token = m_token;
    m_transport.send(m_server, protocol::encode(packet));
}

} // namespace rtype::client
