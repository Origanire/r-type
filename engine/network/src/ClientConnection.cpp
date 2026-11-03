#include "ClientConnection.hpp"

#include <iostream>
#include <utility>

#include "Codec.hpp"
#include "NetError.hpp"

namespace engine::network {

ClientConnection::ClientConnection(net::Endpoint server, const NetworkSettings &settings)
    : m_settings(settings), m_codec(settings), m_server(std::move(server)), m_sequence(0), m_token(0),
      m_attempts(0), m_lastConnect(), m_lastInput(), m_lastHeartbeat(), m_lastReceive(), m_lastTick(),
      m_serverSequence(), m_snapshot(), m_events()
{
}

void ClientConnection::update(std::uint8_t buttons)
{
    const Time now = std::chrono::steady_clock::now();
    for (const net::Datagram &datagram : m_transport.poll()) {
        if (datagram.sender() != m_server)
            continue;
        const auto packet = m_codec.decode(datagram.bytes(), Receiver::Client);
        if (packet && handlePacket(*packet))
            m_lastReceive = now;
    }
    if (!connected()) {
        retryConnect(now);
        return;
    }
    if (now - m_lastReceive >= m_settings.sessionTimeout())
        throw net::NetError("ClientConnection: connection lost, the server stopped answering");
    sendPeriodic(buttons, now);
}

void ClientConnection::disconnect()
{
    if (!connected())
        return;
    send(Packet(DisconnectMessage(m_token)));
    m_token = 0;
}

std::vector<Packet> ClientConnection::takeEvents()
{
    std::vector<Packet> events;
    events.swap(m_events);
    return events;
}

std::optional<SnapshotMessage> ClientConnection::takeSnapshot()
{
    std::optional<SnapshotMessage> snapshot = std::move(m_snapshot);
    m_snapshot.reset();
    return snapshot;
}

bool ClientConnection::connected() const
{
    return m_token != 0;
}

bool ClientConnection::handlePacket(const Packet &packet)
{
    if (!connected())
        return handleConnectionReply(packet);
    if (packet.type() == MessageType::Accept || packet.type() == MessageType::Reject)
        return false;
    if (packet.holds<SnapshotMessage>()) {
        if (!handleSnapshot(packet.as<SnapshotMessage>()))
            return false;
    } else if (m_serverSequence && !Codec::isNewer(packet.sequence(), *m_serverSequence)) {
        return false;
    }
    if (packet.type() == MessageType::ServerShutdown)
        throw net::NetError("ClientConnection: server stopped");
    if (packet.type() == MessageType::EntityDestroyed || packet.type() == MessageType::PlayerKilled)
        m_events.push_back(packet);
    if (!m_serverSequence || Codec::isNewer(packet.sequence(), *m_serverSequence))
        m_serverSequence = packet.sequence();
    return true;
}

bool ClientConnection::handleSnapshot(const SnapshotMessage &snapshot)
{
    if (m_lastTick && !Codec::isNewer(snapshot.tick(), *m_lastTick))
        return false;
    m_lastTick = snapshot.tick();
    m_snapshot = snapshot;
    return true;
}

bool ClientConnection::handleConnectionReply(const Packet &packet)
{
    if (packet.holds<RejectMessage>()) {
        if (packet.as<RejectMessage>().reason() == Protocol::REJECT_SERVER_FULL)
            throw net::NetError("ClientConnection: server full");
        throw net::NetError("ClientConnection: protocol version mismatch");
    }
    if (!packet.holds<AcceptMessage>() || m_attempts == 0)
        return false;
    const AcceptMessage &accept = packet.as<AcceptMessage>();
    m_token = accept.token();
    m_serverSequence = packet.sequence();
    std::cout << "Connected as player " << static_cast<int>(accept.playerId()) << std::endl;
    return true;
}

void ClientConnection::retryConnect(Time now)
{
    if (m_attempts != 0 && now - m_lastConnect < m_settings.connectRetryInterval())
        return;
    if (m_attempts >= m_settings.connectAttempts())
        throw net::NetError("ClientConnection: connection timed out, the server never answered");
    send(Packet(ConnectMessage()));
    ++m_attempts;
    m_lastConnect = now;
}

void ClientConnection::sendPeriodic(std::uint8_t buttons, Time now)
{
    if (now - m_lastInput >= m_settings.inputInterval()) {
        send(Packet(InputMessage(m_token, buttons)));
        m_lastInput = now;
    }
    if (now - m_lastHeartbeat >= m_settings.heartbeatInterval()) {
        const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        send(Packet(HeartbeatMessage(m_token, static_cast<std::uint32_t>(milliseconds.count()))));
        m_lastHeartbeat = now;
    }
}

void ClientConnection::send(Packet packet)
{
    packet.setSequence(m_sequence++);
    m_transport.send(m_server, m_codec.encode(packet));
}

} // namespace engine::network
