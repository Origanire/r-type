#include "GameServer.hpp"

#include <iostream>
#include <utility>

namespace engine::network {

GameServer::GameServer(std::uint16_t port, const NetworkSettings &settings, ISessionFactory &factory)
    : m_settings(settings), m_codec(settings), m_transport(port), m_factory(factory),
      m_clients(settings.maxClients()), m_rooms(), m_random(std::random_device{}()), m_sequence(0),
      m_nextRoomId(1), m_rateWindow(), m_lastRoomUpdate(), m_connectRequests(0)
{
    std::cout << "Server listening on UDP port " << port << std::endl;
    if (const auto address = net::UdpTransport::primaryLocalAddress())
        std::cout << "Players on your network can join with: -h " << *address << " -p " << port << std::endl;
}

void GameServer::poll()
{
    const Time now = std::chrono::steady_clock::now();
    for (std::size_t client = 0; client < m_clients.size(); ++client) {
        if (m_clients[client] && now - m_clients[client]->lastSeen() >= m_settings.sessionTimeout())
            remove(client, Protocol::LEFT_TIMEOUT);
    }
    for (const net::Datagram &datagram : m_transport.poll())
        receive(datagram, now);
}

void GameServer::tick(float dt)
{
    const Time now = std::chrono::steady_clock::now();
    for (auto &entry : m_rooms) {
        if (entry.second.playing())
            tickRoom(entry.second, dt, now);
    }
    if (now - m_lastRoomUpdate < ROOM_UPDATE_INTERVAL)
        return;
    m_lastRoomUpdate = now;
    for (auto &entry : m_rooms)
        sendRoomUpdate(entry.second);
}

void GameServer::shutdown()
{
    for (std::size_t client = 0; client < m_clients.size(); ++client) {
        if (m_clients[client] && m_clients[client]->confirmed())
            sendTo(client, Packet(ServerShutdownMessage()));
    }
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
    const auto client = findClient(datagram.sender());
    if (!client || tokenOf(*packet) != m_clients[*client]->token())
        return;
    handleSessionPacket(*client, *packet, now);
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
    std::optional<std::size_t> client = findClient(datagram.sender());
    if (client) {
        if (now - m_clients[*client]->lastAccept() < ACCEPT_REPEAT_DELAY)
            return;
    } else {
        client = findFreeClient();
        if (!client) {
            reject(datagram.sender(), Protocol::REJECT_SERVER_FULL);
            return;
        }
        m_clients[*client].emplace(datagram.sender(), newToken(), now);
    }
    m_clients[*client]->markAccepted(now);
    send(datagram.sender(), Packet(AcceptMessage(m_clients[*client]->token(), static_cast<std::uint8_t>(*client))));
}

void GameServer::handleSessionPacket(std::size_t client, const Packet &packet, Time now)
{
    if (packet.type() == MessageType::Disconnect) {
        handleDisconnect(client, packet);
        return;
    }
    ClientSession &session = *m_clients[client];
    const bool sequenced = packet.holds<InputMessage>() || packet.holds<HeartbeatMessage>();
    if (sequenced && !acceptSequence(session, packet))
        return;
    session.markSeen(now);
    if (!session.confirmed())
        confirm(client);
    if (packet.holds<InputMessage>())
        handleInput(client, packet.as<InputMessage>(), now);
    else if (packet.holds<HeartbeatMessage>())
        sendTo(client, Packet(HeartbeatAckMessage(packet.as<HeartbeatMessage>().timestamp())));
    else if (packet.holds<ListRoomsMessage>())
        sendRoomList(client);
    else if (packet.holds<CreateRoomMessage>())
        handleCreateRoom(client, packet.as<CreateRoomMessage>());
    else if (packet.holds<JoinRoomMessage>())
        handleJoinRoom(client, packet.as<JoinRoomMessage>());
    else if (packet.holds<LeaveRoomMessage>())
        leaveRoom(client);
    else if (packet.holds<StartGameMessage>())
        handleStartGame(client);
    else if (packet.holds<KickPlayerMessage>())
        handleKick(client, packet.as<KickPlayerMessage>());
    else if (packet.holds<DeleteRoomMessage>())
        handleDeleteRoom(client);
    else if (packet.holds<ConsoleCommandMessage>())
        handleConsole(client, packet.as<ConsoleCommandMessage>());
}

void GameServer::handleDisconnect(std::size_t client, const Packet &packet)
{
    const ClientSession &session = *m_clients[client];
    const auto input = session.inputSequence();
    const auto heartbeat = session.heartbeatSequence();
    const bool staleInput = input && !Codec::isNewer(packet.sequence(), *input);
    const bool staleHeartbeat = heartbeat && !Codec::isNewer(packet.sequence(), *heartbeat);
    if (!staleInput && !staleHeartbeat)
        remove(client, Protocol::LEFT_QUIT);
}

bool GameServer::acceptSequence(ClientSession &session, const Packet &packet)
{
    const bool input = packet.holds<InputMessage>();
    const auto previous = input ? session.inputSequence() : session.heartbeatSequence();
    if (previous && !Codec::isNewer(packet.sequence(), *previous))
        return false;
    if (input)
        session.setInputSequence(packet.sequence());
    else
        session.setHeartbeatSequence(packet.sequence());
    return true;
}

void GameServer::handleInput(std::size_t client, const InputMessage &input, Time now)
{
    m_clients[client]->markInput(now);
    GameRoom *room = roomOf(client);
    if (!room || !room->playing())
        return;
    if (const auto slot = room->slotOf(client))
        room->session().setInput(*slot, input.buttons());
}

void GameServer::handleCreateRoom(std::size_t client, const CreateRoomMessage &request)
{
    if (GameRoom *current = roomOf(client)) {
        sendRoomJoined(client, *current);
        return;
    }
    if (m_rooms.size() >= m_settings.maxRooms()) {
        sendTo(client, Packet(RoomRejectedMessage(Protocol::ROOM_LIMIT_REACHED)));
        return;
    }
    const std::uint16_t id = newRoomId();
    GameRoom &room = m_rooms
                         .try_emplace(id, id, request.name(), request.code(), m_settings.maxPlayers(),
                             m_factory.createSession(request.rules()), request.rules())
                         .first->second;
    seatClient(client, room, GameRoom::OWNER_SLOT);
    std::cout << "Client " << client << " created room " << id << " (" << request.name() << ")" << std::endl;
}

void GameServer::handleJoinRoom(std::size_t client, const JoinRoomMessage &request)
{
    if (GameRoom *current = roomOf(client)) {
        sendRoomJoined(client, *current);
        return;
    }
    ClientSession &session = *m_clients[client];
    const auto found = m_rooms.find(request.roomId());
    std::uint8_t reason = 0;
    if (session.failedJoins() >= m_settings.maxJoinAttempts())
        reason = Protocol::ROOM_TOO_MANY_ATTEMPTS;
    else if (found == m_rooms.end())
        reason = Protocol::ROOM_NOT_FOUND;
    else if (!found->second.freeSlot())
        reason = Protocol::ROOM_FULL;
    else if (!found->second.acceptsCode(request.code()))
        reason = Protocol::ROOM_WRONG_CODE;
    if (reason == Protocol::ROOM_WRONG_CODE)
        session.addFailedJoin();
    if (reason != 0) {
        sendTo(client, Packet(RoomRejectedMessage(reason)));
        return;
    }
    GameRoom &room = found->second;
    seatClient(client, room, *room.freeSlot());
}

void GameServer::handleStartGame(std::size_t client)
{
    GameRoom *room = roomOf(client);
    if (!ownedRoomSlot(client, room) || room->playing())
        return;
    room->setPlaying(true);
    for (std::size_t slot = 0; slot < room->capacity(); ++slot) {
        if (room->clientAt(slot))
            room->session().addPlayer(slot);
    }
    for (const auto &[slot, text] : room->takeCommands())
        room->session().command(slot, text);
    broadcastRoom(*room, Packet(GameStartedMessage()));
    std::cout << "Room " << room->id() << " started" << std::endl;
}

void GameServer::handleKick(std::size_t client, const KickPlayerMessage &request)
{
    GameRoom *room = roomOf(client);
    const auto ownerSlot = ownedRoomSlot(client, room);
    if (!ownerSlot || request.slot() == *ownerSlot)
        return;
    const auto target = room->clientAt(request.slot());
    if (!target)
        return;
    sendTo(*target, Packet(KickedMessage()));
    leaveRoom(*target);
}

void GameServer::handleConsole(std::size_t client, const ConsoleCommandMessage &request)
{
    GameRoom *room = roomOf(client);
    const auto slot = room ? room->slotOf(client) : std::nullopt;
    if (!slot || *slot != room->owner()) {
        sendTo(client, Packet(ConsoleTextMessage("server commands are reserved to the room owner")));
        return;
    }
    if (const auto rule = m_settings.consoleRule(); rule && room->rule(*rule) == 0) {
        sendTo(client, Packet(ConsoleTextMessage("server commands are disabled in this room (cheats OFF)")));
        return;
    }
    if (!room->playing()) {
        room->queueCommand(*slot, request.text());
        sendTo(client, Packet(ConsoleTextMessage("queued: runs when the game starts")));
        return;
    }
    room->session().command(*slot, request.text());
}

void GameServer::handleDeleteRoom(std::size_t client)
{
    GameRoom *room = roomOf(client);
    if (!ownedRoomSlot(client, room))
        return;
    broadcastRoom(*room, Packet(RoomClosedMessage()));
    closeRoom(room->id());
}

void GameServer::confirm(std::size_t client)
{
    m_clients[client]->confirm();
    std::cout << "Client " << client << " connected" << std::endl;
}

void GameServer::remove(std::size_t client, std::uint8_t reason)
{
    leaveRoom(client);
    const bool confirmed = m_clients[client]->confirmed();
    m_clients[client].reset();
    if (confirmed)
        std::cout << "Client " << client << " left (reason " << static_cast<int>(reason) << ")" << std::endl;
}

void GameServer::tickRoom(GameRoom &room, float dt, Time now)
{
    for (std::size_t slot = 0; slot < room.capacity(); ++slot) {
        const auto client = room.clientAt(slot);
        if (client && now - m_clients[*client]->lastInput() >= m_settings.inputReleaseDelay())
            room.session().setInput(slot, 0);
    }
    room.session().update(dt);
    for (const Packet &event : room.session().takeEvents())
        broadcastRoom(room, event);
    if (room.session().tick() % m_settings.snapshotEveryTicks() == 0)
        broadcastRoom(room, Packet(room.session().snapshot()));
}

void GameServer::seatClient(std::size_t client, GameRoom &room, std::size_t slot)
{
    room.seat(slot, client);
    m_clients[client]->enterRoom(room.id());
    if (room.playing()) {
        room.session().addPlayer(slot);
        sendRoomJoined(client, room);
        sendTo(client, Packet(GameStartedMessage()));
    } else {
        sendRoomJoined(client, room);
    }
    sendRoomUpdate(room);
}

void GameServer::leaveRoom(std::size_t client)
{
    GameRoom *room = roomOf(client);
    m_clients[client]->leaveRoom();
    if (!room)
        return;
    const auto slot = room->slotOf(client);
    if (!slot)
        return;
    if (*slot == room->owner()) {
        room->unseat(*slot);
        broadcastRoom(*room, Packet(RoomClosedMessage()));
        closeRoom(room->id());
        return;
    }
    if (room->playing())
        room->session().removePlayer(*slot);
    room->unseat(*slot);
    sendRoomUpdate(*room);
}

void GameServer::closeRoom(std::uint16_t roomId)
{
    const auto found = m_rooms.find(roomId);
    if (found == m_rooms.end())
        return;
    for (std::size_t member : found->second.clients()) {
        if (m_clients[member])
            m_clients[member]->leaveRoom();
    }
    m_rooms.erase(found);
    std::cout << "Room " << roomId << " closed" << std::endl;
}

GameRoom *GameServer::roomOf(std::size_t client)
{
    if (!m_clients[client])
        return nullptr;
    const auto roomId = m_clients[client]->roomId();
    if (!roomId)
        return nullptr;
    const auto found = m_rooms.find(*roomId);
    return found == m_rooms.end() ? nullptr : &found->second;
}

std::optional<std::size_t> GameServer::ownedRoomSlot(std::size_t client, GameRoom *room)
{
    if (!room)
        return std::nullopt;
    const auto slot = room->slotOf(client);
    if (!slot || *slot != room->owner()) {
        sendTo(client, Packet(RoomRejectedMessage(Protocol::ROOM_NOT_ALLOWED)));
        return std::nullopt;
    }
    return slot;
}

std::uint16_t GameServer::newRoomId()
{
    while (m_nextRoomId == 0 || m_rooms.contains(m_nextRoomId))
        ++m_nextRoomId;
    return m_nextRoomId++;
}

void GameServer::sendRoomList(std::size_t client)
{
    std::vector<RoomInfo> rooms;
    for (const auto &entry : m_rooms) {
        const GameRoom &room = entry.second;
        rooms.emplace_back(room.id(), room.name(), static_cast<std::uint8_t>(room.playerCount()),
            static_cast<std::uint8_t>(room.capacity()), room.locked(), room.playing());
    }
    sendTo(client, Packet(RoomListMessage(std::move(rooms))));
}

void GameServer::sendRoomJoined(std::size_t client, GameRoom &room)
{
    const auto slot = room.slotOf(client);
    if (slot)
        sendTo(client, Packet(RoomJoinedMessage(room.id(), room.name(), static_cast<std::uint8_t>(*slot))));
}

void GameServer::sendRoomUpdate(GameRoom &room)
{
    broadcastRoom(room, Packet(RoomUpdateMessage(static_cast<std::uint8_t>(room.owner()), room.occupiedSlots())));
}

void GameServer::broadcastRoom(GameRoom &room, const Packet &packet)
{
    for (std::size_t client : room.clients())
        sendTo(client, packet);
}

void GameServer::sendTo(std::size_t client, Packet packet)
{
    if (m_clients[client])
        send(m_clients[client]->peer(), std::move(packet));
}

void GameServer::send(const net::Endpoint &peer, Packet packet)
{
    packet.setSequence(m_sequence++);
    m_transport.send(peer, m_codec.encode(packet));
}

void GameServer::reject(const net::Endpoint &peer, std::uint8_t reason)
{
    send(peer, Packet(RejectMessage(reason)));
}

std::optional<std::size_t> GameServer::findClient(const net::Endpoint &peer) const
{
    for (std::size_t client = 0; client < m_clients.size(); ++client) {
        if (m_clients[client] && m_clients[client]->peer() == peer)
            return client;
    }
    return std::nullopt;
}

std::optional<std::size_t> GameServer::findFreeClient() const
{
    for (std::size_t client = 0; client < m_clients.size(); ++client) {
        if (!m_clients[client])
            return client;
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
    if (packet.holds<ListRoomsMessage>())
        return packet.as<ListRoomsMessage>().token();
    if (packet.holds<CreateRoomMessage>())
        return packet.as<CreateRoomMessage>().token();
    if (packet.holds<JoinRoomMessage>())
        return packet.as<JoinRoomMessage>().token();
    if (packet.holds<LeaveRoomMessage>())
        return packet.as<LeaveRoomMessage>().token();
    if (packet.holds<StartGameMessage>())
        return packet.as<StartGameMessage>().token();
    if (packet.holds<KickPlayerMessage>())
        return packet.as<KickPlayerMessage>().token();
    if (packet.holds<DeleteRoomMessage>())
        return packet.as<DeleteRoomMessage>().token();
    if (packet.holds<ConsoleCommandMessage>())
        return packet.as<ConsoleCommandMessage>().token();
    return std::nullopt;
}

} // namespace engine::network
