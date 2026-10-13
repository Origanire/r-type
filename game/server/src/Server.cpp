#include "Server.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "Position.hpp"
#include "WorldSize.hpp"

namespace rtype::server {
using protocol::MessageType;
using namespace std::chrono_literals;

Server::Server(std::uint16_t port) : m_transport(port), m_random(std::random_device{}())
{
    m_registry.registerComponent<Position>();
    std::cout << "R-Type server listening on UDP port " << port << std::endl;
}

void Server::send(const engine::net::Endpoint &peer, protocol::Packet packet)
{
    packet.sequence = m_sequence++;
    m_transport.send(peer, protocol::encode(packet));
}

void Server::receive(const engine::net::Datagram &datagram, Time now)
{
    auto packet = protocol::decode(datagram.bytes, protocol::Receiver::Server);
    if (!packet)
        return;
    if (packet->type == MessageType::Connect) {
        if (now - m_rateWindow >= 1s) {
            m_rateWindow = now;
            m_connectRequests = 0;
        }
        if (++m_connectRequests > 32)
            return;
        protocol::Packet reply;
        if (packet->version != protocol::VERSION) {
            reply.type = MessageType::Reject; reply.reason = 2;
            send(datagram.sender, reply);
            return;
        }
        for (std::size_t i = 0; i < m_sessions.size(); ++i) {
            if (m_sessions[i] && m_sessions[i]->peer == datagram.sender) {
                if (now - m_sessions[i]->lastAccept < 200ms)
                    return;
                m_sessions[i]->lastAccept = now;
                reply.type = MessageType::Accept; reply.token = m_sessions[i]->token;
                reply.playerId = static_cast<std::uint8_t>(i);
                send(datagram.sender, reply);
                return;
            }
        }
        for (std::size_t i = 0; i < m_sessions.size(); ++i) {
            if (m_sessions[i])
                continue;
            std::uint32_t token;
            bool duplicate;
            do {
                token = static_cast<std::uint32_t>(m_random());
                duplicate = token == 0;
                for (const auto &session : m_sessions)
                    duplicate = duplicate || (session && session->token == token);
            } while (duplicate);
            Session session{datagram.sender, token, std::nullopt, now, now, now, std::nullopt, std::nullopt};
            m_sessions[i] = session;
            reply.type = MessageType::Accept; reply.token = token; reply.playerId = static_cast<std::uint8_t>(i);
            send(datagram.sender, reply);
            return;
        }
        reply.type = MessageType::Reject; reply.reason = 1;
        send(datagram.sender, reply);
        return;
    }
    for (std::size_t i = 0; i < m_sessions.size(); ++i) {
        auto &session = m_sessions[i];
        if (!session || session->peer != datagram.sender || session->token != packet->token)
            continue;
        if (packet->type == MessageType::Disconnect) {
            if ((session->inputSequence && !protocol::isNewer(packet->sequence, *session->inputSequence)) ||
                (session->heartbeatSequence && !protocol::isNewer(packet->sequence, *session->heartbeatSequence)))
                return;
            remove(i, 1);
            return;
        }
        auto &sequence = packet->type == MessageType::Input ? session->inputSequence : session->heartbeatSequence;
        if (sequence && !protocol::isNewer(packet->sequence, *sequence))
            return;
        sequence = packet->sequence;
        session->lastSeen = now;
        if (!session->confirmed) {
            session->confirmed = true;
            session->ship = m_registry.spawnEntity();
            m_registry.addComponent(*session->ship, Position{200.f, 200.f + static_cast<float>(i) * 160.f});
            // Never reuse a live network id, including across u16 wraparound.
            while (m_nextId == 0 || std::find(m_ids.begin(), m_ids.end(), m_nextId) != m_ids.end())
                ++m_nextId;
            m_ids[i] = m_nextId++;
            protocol::Packet joined; joined.type = MessageType::PlayerJoined; joined.playerId = static_cast<std::uint8_t>(i);
            for (const auto &other : m_sessions)
                if (other && other->confirmed)
                    send(other->peer, joined);
            std::cout << "Player " << i << " joined" << std::endl;
        }
        if (packet->type == MessageType::Input) {
            session->buttons = packet->buttons;
            session->lastInput = now;
        } else if (packet->type == MessageType::Heartbeat) {
            protocol::Packet reply; reply.type = MessageType::HeartbeatAck; reply.timestamp = packet->timestamp;
            send(session->peer, reply);
        }
        return;
    }
}

void Server::remove(std::size_t slot, std::uint8_t reason)
{
    if (m_sessions[slot]->ship)
        m_registry.killEntity(*m_sessions[slot]->ship);
    const bool confirmed = m_sessions[slot]->confirmed;
    m_sessions[slot].reset();
    m_ids[slot] = 0;
    if (!confirmed)
        return;
    protocol::Packet left; left.type = MessageType::PlayerLeft;
    left.playerId = static_cast<std::uint8_t>(slot); left.reason = reason;
    for (const auto &other : m_sessions)
        if (other && other->confirmed)
            send(other->peer, left);
    std::cout << "Player " << slot << " left (reason " << static_cast<int>(reason) << ")" << std::endl;
}

void Server::step(Time now)
{
    for (std::size_t i = 0; i < m_sessions.size(); ++i)
        if (m_sessions[i] && now - m_sessions[i]->lastSeen >= 5s)
            remove(i, 2);
    for (const auto &datagram : m_transport.poll())
        receive(datagram, now);
    for (auto &session : m_sessions) {
        if (!session || !session->confirmed)
            continue;
        if (now - session->lastInput >= 500ms)
            session->buttons = 0;
        auto &position = *m_registry.getComponents<Position>()[*session->ship];
        const int bits = session->buttons;
        // Match the local prototype's 600 units/s and 99 x 51 ship dimensions.
        position.x = std::clamp(position.x + ((bits & 8 ? 1.f : 0.f) - (bits & 4 ? 1.f : 0.f)) * 10.f,
            0.f, WORLD_WIDTH - 99.f);
        position.y = std::clamp(position.y + ((bits & 2 ? 1.f : 0.f) - (bits & 1 ? 1.f : 0.f)) * 10.f,
            0.f, WORLD_HEIGHT - 51.f);
    }
    ++m_tick;
    if (m_tick % 3 == 0)
        snapshots();
}

void Server::snapshots()
{
    protocol::Packet packet; packet.type = MessageType::Snapshot; packet.snapshot.tick = m_tick;
    for (std::size_t i = 0; i < m_sessions.size(); ++i) {
        if (!m_sessions[i] || !m_sessions[i]->confirmed)
            continue;
        packet.snapshot.gameState = 1;
        packet.snapshot.players[i] = {1, 3, 0, m_ids[i]};
        const auto &position = *m_registry.getComponents<Position>()[*m_sessions[i]->ship];
        packet.snapshot.entities.push_back({m_ids[i], 1,
            static_cast<std::int16_t>(std::lround(position.x)), static_cast<std::int16_t>(std::lround(position.y))});
    }
    for (const auto &session : m_sessions)
        if (session && session->confirmed)
            send(session->peer, packet);
}

void Server::shutdown()
{
    protocol::Packet packet; packet.type = MessageType::ServerShutdown;
    for (const auto &session : m_sessions)
        if (session && session->confirmed)
            send(session->peer, packet);
}

} // namespace rtype::server
