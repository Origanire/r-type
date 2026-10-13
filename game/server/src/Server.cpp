#include "Server.hpp"

#include <iostream>

#include "GameConstants.hpp"

namespace rtype::server {

using protocol::MessageType;

namespace {

constexpr unsigned MAX_CONNECT_REQUESTS_PER_WINDOW = 32;
constexpr std::chrono::seconds CONNECT_RATE_WINDOW{1};
constexpr std::chrono::milliseconds ACCEPT_REPEAT_DELAY{200};

} // namespace

Server::Server(std::uint16_t port) : m_transport(port), m_random(std::random_device{}())
{
    std::cout << "R-Type server listening on UDP port " << port << std::endl;
}

void Server::poll()
{
    const Time now = std::chrono::steady_clock::now();
    for (std::size_t slot = 0; slot < m_sessions.size(); ++slot) {
        if (m_sessions[slot] && now - m_sessions[slot]->lastSeen >= protocol::SESSION_TIMEOUT)
            remove(slot, protocol::LEFT_TIMEOUT);
    }
    for (const auto &datagram : m_transport.poll())
        receive(datagram, now);
}

void Server::tick(float dt)
{
    const Time now = std::chrono::steady_clock::now();
    for (std::size_t slot = 0; slot < m_sessions.size(); ++slot) {
        if (m_sessions[slot] && now - m_sessions[slot]->lastInput >= protocol::INPUT_RELEASE_DELAY)
            m_room.setButtons(slot, 0);
    }
    m_room.update(dt);
    if (m_room.tick() % protocol::SNAPSHOT_INTERVAL_TICKS == 0)
        sendSnapshots();
}

void Server::shutdown()
{
    protocol::Packet packet;
    packet.type = MessageType::ServerShutdown;
    broadcast(packet);
}

void Server::receive(const engine::net::Datagram &datagram, Time now)
{
    const auto packet = protocol::decode(datagram.bytes, protocol::Receiver::Server);
    if (!packet)
        return;
    if (packet->type == MessageType::Connect) {
        handleConnect(datagram, *packet, now);
        return;
    }
    const auto slot = findSession(datagram.sender);
    if (!slot || m_sessions[*slot]->token != packet->token)
        return;
    handleSessionPacket(*slot, *packet, now);
}

void Server::handleConnect(const engine::net::Datagram &datagram, const protocol::Packet &packet, Time now)
{
    if (now - m_rateWindow >= CONNECT_RATE_WINDOW) {
        m_rateWindow = now;
        m_connectRequests = 0;
    }
    if (++m_connectRequests > MAX_CONNECT_REQUESTS_PER_WINDOW)
        return;
    if (packet.version != protocol::VERSION) {
        reject(datagram.sender, protocol::REJECT_VERSION_MISMATCH);
        return;
    }
    std::optional<std::size_t> slot = findSession(datagram.sender);
    if (slot) {
        if (now - m_sessions[*slot]->lastAccept < ACCEPT_REPEAT_DELAY)
            return;
    } else {
        slot = findFreeSlot();
        if (!slot) {
            reject(datagram.sender, protocol::REJECT_SERVER_FULL);
            return;
        }
        Session session;
        session.peer = datagram.sender;
        session.token = newToken();
        session.lastSeen = now;
        session.lastInput = now;
        m_sessions[*slot] = session;
    }
    m_sessions[*slot]->lastAccept = now;
    protocol::Packet accept;
    accept.type = MessageType::Accept;
    accept.token = m_sessions[*slot]->token;
    accept.playerId = static_cast<std::uint8_t>(*slot);
    send(datagram.sender, accept);
}

void Server::handleSessionPacket(std::size_t slot, const protocol::Packet &packet, Time now)
{
    Session &session = *m_sessions[slot];
    if (packet.type == MessageType::Disconnect) {
        const bool staleInput = session.inputSequence && !protocol::isNewer(packet.sequence, *session.inputSequence);
        const bool staleHeartbeat =
            session.heartbeatSequence && !protocol::isNewer(packet.sequence, *session.heartbeatSequence);
        if (!staleInput && !staleHeartbeat)
            remove(slot, protocol::LEFT_QUIT);
        return;
    }
    auto &sequence = packet.type == MessageType::Input ? session.inputSequence : session.heartbeatSequence;
    if (sequence && !protocol::isNewer(packet.sequence, *sequence))
        return;
    sequence = packet.sequence;
    session.lastSeen = now;
    if (!session.confirmed)
        confirm(slot);
    if (packet.type == MessageType::Input) {
        m_room.setButtons(slot, packet.buttons);
        session.lastInput = now;
    } else if (packet.type == MessageType::Heartbeat) {
        protocol::Packet ack;
        ack.type = MessageType::HeartbeatAck;
        ack.timestamp = packet.timestamp;
        send(session.peer, ack);
    }
}

void Server::confirm(std::size_t slot)
{
    m_sessions[slot]->confirmed = true;
    m_room.spawnPlayer(slot);
    protocol::Packet joined;
    joined.type = MessageType::PlayerJoined;
    joined.playerId = static_cast<std::uint8_t>(slot);
    broadcast(joined);
    std::cout << "Player " << slot << " joined" << std::endl;
}

void Server::remove(std::size_t slot, std::uint8_t reason)
{
    const bool confirmed = m_sessions[slot]->confirmed;
    m_room.removePlayer(slot);
    m_sessions[slot].reset();
    if (!confirmed)
        return;
    protocol::Packet left;
    left.type = MessageType::PlayerLeft;
    left.playerId = static_cast<std::uint8_t>(slot);
    left.reason = reason;
    broadcast(left);
    std::cout << "Player " << slot << " left (reason " << static_cast<int>(reason) << ")" << std::endl;
}

void Server::sendSnapshots()
{
    protocol::Packet packet;
    packet.type = MessageType::Snapshot;
    packet.snapshot.tick = m_room.tick();
    packet.snapshot.entities = m_room.networkEntities();
    for (std::size_t slot = 0; slot < m_sessions.size(); ++slot) {
        if (!m_sessions[slot] || !m_sessions[slot]->confirmed)
            continue;
        packet.snapshot.gameState = protocol::GAME_PLAYING;
        packet.snapshot.players[slot] = {protocol::SLOT_ALIVE, PLAYER_START_LIVES, 0, m_room.playerNetworkId(slot)};
    }
    broadcast(packet);
}

void Server::send(const engine::net::Endpoint &peer, protocol::Packet packet)
{
    packet.sequence = m_sequence++;
    m_transport.send(peer, protocol::encode(packet));
}

void Server::broadcast(const protocol::Packet &packet)
{
    for (const auto &session : m_sessions) {
        if (session && session->confirmed)
            send(session->peer, packet);
    }
}

void Server::reject(const engine::net::Endpoint &peer, std::uint8_t reason)
{
    protocol::Packet packet;
    packet.type = MessageType::Reject;
    packet.reason = reason;
    send(peer, packet);
}

std::optional<std::size_t> Server::findSession(const engine::net::Endpoint &peer) const
{
    for (std::size_t slot = 0; slot < m_sessions.size(); ++slot) {
        if (m_sessions[slot] && m_sessions[slot]->peer == peer)
            return slot;
    }
    return std::nullopt;
}

std::optional<std::size_t> Server::findFreeSlot() const
{
    for (std::size_t slot = 0; slot < m_sessions.size(); ++slot) {
        if (!m_sessions[slot])
            return slot;
    }
    return std::nullopt;
}

std::uint32_t Server::newToken()
{
    while (true) {
        const auto token = static_cast<std::uint32_t>(m_random());
        bool used = token == 0;
        for (const auto &session : m_sessions) {
            if (session && session->token == token)
                used = true;
        }
        if (!used)
            return token;
    }
}

} // namespace rtype::server
