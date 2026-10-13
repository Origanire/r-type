#include "NetworkClient.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace rtype::client {
using protocol::MessageType;
using namespace std::chrono_literals;

NetworkClient::NetworkClient(engine::net::Endpoint server) : m_server(std::move(server)) {}

void NetworkClient::send(protocol::Packet packet)
{
    packet.sequence = m_sequence++;
    packet.token = m_token;
    m_transport.send(m_server, protocol::encode(packet));
}

bool NetworkClient::connected() const { return m_token != 0; }

void NetworkClient::update(std::uint8_t buttons)
{
    const auto now = std::chrono::steady_clock::now();
    for (const auto &datagram : m_transport.poll()) {
        if (datagram.sender != m_server)
            continue;
        const auto packet = protocol::decode(datagram.bytes, protocol::Receiver::Client);
        if (!packet)
            continue;
        if (!connected()) {
            if (packet->type == MessageType::Reject)
                throw std::runtime_error(packet->reason == 1 ? "Server full" : "Protocol version mismatch");
            if (packet->type != MessageType::Accept || m_attempts == 0)
                continue;
            m_token = packet->token;
            m_serverSequence = packet->sequence;
            std::cout << "Connected as player " << static_cast<int>(packet->playerId) << std::endl;
        } else {
            // A late ACCEPT must not replace an established session.
            if (packet->type == MessageType::Accept || packet->type == MessageType::Reject)
                continue;
            if (packet->type == MessageType::Snapshot) {
                if (m_lastTick && !protocol::isNewer(packet->snapshot.tick, *m_lastTick))
                    continue;
                m_lastTick = packet->snapshot.tick;
                m_snapshot = packet->snapshot;
            } else if (m_serverSequence && !protocol::isNewer(packet->sequence, *m_serverSequence))
                continue;
            if (packet->type == MessageType::ServerShutdown)
                throw std::runtime_error("Server stopped");
            if (!m_serverSequence || protocol::isNewer(packet->sequence, *m_serverSequence))
                m_serverSequence = packet->sequence;
        }
        m_lastReceive = now;
    }
    if (!connected()) {
        if (m_attempts == 0 || now - m_lastConnect >= 1s) {
            if (m_attempts >= 5)
                throw std::runtime_error("Connection timed out after five attempts");
            protocol::Packet packet; packet.type = MessageType::Connect;
            send(packet); ++m_attempts; m_lastConnect = now;
        }
        return;
    }
    if (now - m_lastReceive >= 5s)
        throw std::runtime_error("Connection lost: no server packet for five seconds");
    if (now - m_lastInput >= 33333us) {
        protocol::Packet packet; packet.type = MessageType::Input; packet.buttons = buttons;
        send(packet); m_lastInput = now;
    }
    if (now - m_lastHeartbeat >= 1s) {
        protocol::Packet packet; packet.type = MessageType::Heartbeat;
        packet.timestamp = static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
        send(packet); m_lastHeartbeat = now;
    }
}

void NetworkClient::disconnect()
{
    if (!connected())
        return;
    protocol::Packet packet; packet.type = MessageType::Disconnect;
    send(packet); m_token = 0;
}

std::optional<protocol::WorldSnapshot> NetworkClient::takeSnapshot()
{
    auto snapshot = std::move(m_snapshot);
    m_snapshot.reset();
    return snapshot;
}

} // namespace rtype::client
