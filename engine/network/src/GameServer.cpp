#include "GameServer.hpp"

#include <iostream>

namespace engine::network {

GameServer::GameServer(std::uint16_t port, const NetworkSettings &settings, IGameSession &session)
    : m_settings(settings), m_codec(settings), m_transport(port), m_session(session),
      m_clients(settings.maxPlayers()), m_random(std::random_device{}()), m_sequence(0), m_rateWindow(),
      m_connectRequests(0)
{
    std::cout << "Server listening on UDP port " << port << std::endl;
}

void GameServer::poll()
{
    const Time now = std::chrono::steady_clock::now();
    for (std::size_t slot = 0; slot < m_clients.size(); ++slot) {
        if (m_clients[slot] && now - m_clients[slot]->lastSeen() >= m_settings.sessionTimeout())
            remove(slot, Protocol::LEFT_TIMEOUT);
    }
    for (const net::Datagram &datagram : m_transport.poll())
        receive(datagram, now);
}

void GameServer::tick(float dt)
{
    const Time now = std::chrono::steady_clock::now();
    for (std::size_t slot = 0; slot < m_clients.size(); ++slot) {
        if (m_clients[slot] && now - m_clients[slot]->lastInput() >= m_settings.inputReleaseDelay())
            m_session.setInput(slot, 0);
    }
    m_session.update(dt);
    for (const Packet &event : m_session.takeEvents())
        broadcast(event);
    if (m_session.tick() % m_settings.snapshotEveryTicks() == 0)
        broadcast(Packet(m_session.snapshot()));
}

void GameServer::shutdown()
{
    broadcast(Packet(ServerShutdownMessage()));
}

void GameServer::receive(const net::Datagram &datagram, Time now)
{
    const auto packet = m_codec.decode(datagram.bytes(), Receiver::Server);
    if (!packet)
        return;
    if (packet->type() == MessageType::Connect) {
        handleConnect(datagram, *packet, now);
        return;
    }
    const auto slot = findSession(datagram.sender());
    if (!slot || tokenOf(*packet) != m_clients[*slot]->token())
        return;
    handleSessionPacket(*slot, *packet, now);
}

void GameServer::handleConnect(const net::Datagram &datagram, const Packet &packet, Time now)
{
    if (now - m_rateWindow >= CONNECT_RATE_WINDOW) {
        m_rateWindow = now;
        m_connectRequests = 0;
    }
    if (++m_connectRequests > MAX_CONNECT_REQUESTS_PER_WINDOW)
        return;
    if (packet.version() != Protocol::VERSION) {
        reject(datagram.sender(), Protocol::REJECT_VERSION_MISMATCH);
        return;
    }
    std::optional<std::size_t> slot = findSession(datagram.sender());
    if (slot) {
        if (now - m_clients[*slot]->lastAccept() < ACCEPT_REPEAT_DELAY)
            return;
    } else {
        slot = findFreeSlot();
        if (!slot) {
            reject(datagram.sender(), Protocol::REJECT_SERVER_FULL);
            return;
        }
        m_clients[*slot].emplace(datagram.sender(), newToken(), now);
    }
    m_clients[*slot]->markAccepted(now);
    send(datagram.sender(), Packet(AcceptMessage(m_clients[*slot]->token(), static_cast<std::uint8_t>(*slot))));
}

void GameServer::handleSessionPacket(std::size_t slot, const Packet &packet, Time now)
{
    if (packet.type() == MessageType::Disconnect) {
        handleDisconnect(slot, packet);
        return;
    }
    ClientSession &client = *m_clients[slot];
    if (!acceptSequence(client, packet))
        return;
    client.markSeen(now);
    if (!client.confirmed())
        confirm(slot);
    if (packet.holds<InputMessage>()) {
        m_session.setInput(slot, packet.as<InputMessage>().buttons());
        client.markInput(now);
    } else if (packet.holds<HeartbeatMessage>()) {
        send(client.peer(), Packet(HeartbeatAckMessage(packet.as<HeartbeatMessage>().timestamp())));
    }
}

void GameServer::handleDisconnect(std::size_t slot, const Packet &packet)
{
    const ClientSession &client = *m_clients[slot];
    const auto input = client.inputSequence();
    const auto heartbeat = client.heartbeatSequence();
    const bool staleInput = input && !Codec::isNewer(packet.sequence(), *input);
    const bool staleHeartbeat = heartbeat && !Codec::isNewer(packet.sequence(), *heartbeat);
    if (!staleInput && !staleHeartbeat)
        remove(slot, Protocol::LEFT_QUIT);
}

bool GameServer::acceptSequence(ClientSession &client, const Packet &packet)
{
    const bool input = packet.holds<InputMessage>();
    const auto previous = input ? client.inputSequence() : client.heartbeatSequence();
    if (previous && !Codec::isNewer(packet.sequence(), *previous))
        return false;
    if (input)
        client.setInputSequence(packet.sequence());
    else
        client.setHeartbeatSequence(packet.sequence());
    return true;
}

void GameServer::confirm(std::size_t slot)
{
    m_clients[slot]->confirm();
    m_session.addPlayer(slot);
    broadcast(Packet(PlayerJoinedMessage(static_cast<std::uint8_t>(slot))));
    std::cout << "Player " << slot << " joined" << std::endl;
}

void GameServer::remove(std::size_t slot, std::uint8_t reason)
{
    const bool confirmed = m_clients[slot]->confirmed();
    m_session.removePlayer(slot);
    m_clients[slot].reset();
    if (!confirmed)
        return;
    broadcast(Packet(PlayerLeftMessage(static_cast<std::uint8_t>(slot), reason)));
    std::cout << "Player " << slot << " left (reason " << static_cast<int>(reason) << ")" << std::endl;
}

void GameServer::send(const net::Endpoint &peer, Packet packet)
{
    packet.setSequence(m_sequence++);
    m_transport.send(peer, m_codec.encode(packet));
}

void GameServer::broadcast(const Packet &packet)
{
    for (const auto &client : m_clients) {
        if (client && client->confirmed())
            send(client->peer(), packet);
    }
}

void GameServer::reject(const net::Endpoint &peer, std::uint8_t reason)
{
    send(peer, Packet(RejectMessage(reason)));
}

std::optional<std::size_t> GameServer::findSession(const net::Endpoint &peer) const
{
    for (std::size_t slot = 0; slot < m_clients.size(); ++slot) {
        if (m_clients[slot] && m_clients[slot]->peer() == peer)
            return slot;
    }
    return std::nullopt;
}

std::optional<std::size_t> GameServer::findFreeSlot() const
{
    for (std::size_t slot = 0; slot < m_clients.size(); ++slot) {
        if (!m_clients[slot])
            return slot;
    }
    return std::nullopt;
}

std::uint32_t GameServer::newToken()
{
    while (true) {
        const auto token = static_cast<std::uint32_t>(m_random());
        bool used = token == 0;
        for (const auto &client : m_clients) {
            if (client && client->token() == token)
                used = true;
        }
        if (!used)
            return token;
    }
}

std::optional<std::uint32_t> GameServer::tokenOf(const Packet &packet)
{
    if (packet.holds<InputMessage>())
        return packet.as<InputMessage>().token();
    if (packet.holds<HeartbeatMessage>())
        return packet.as<HeartbeatMessage>().token();
    if (packet.holds<DisconnectMessage>())
        return packet.as<DisconnectMessage>().token();
    return std::nullopt;
}

} // namespace engine::network
