#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

#include "ClientSession.hpp"
#include "Codec.hpp"
#include "Datagram.hpp"
#include "Endpoint.hpp"
#include "IGameSession.hpp"
#include "NetworkSettings.hpp"
#include "Packet.hpp"
#include "UdpTransport.hpp"

namespace engine::network {

class GameServer
{
public:
    static constexpr unsigned MAX_CONNECT_REQUESTS_PER_WINDOW = 32;
    static constexpr std::chrono::seconds CONNECT_RATE_WINDOW{1};
    static constexpr std::chrono::milliseconds ACCEPT_REPEAT_DELAY{200};

    GameServer(std::uint16_t port, const NetworkSettings &settings, IGameSession &session);

    void poll();
    void tick(float dt);
    void shutdown();

private:
    using Time = ClientSession::Time;

    void receive(const net::Datagram &datagram, Time now);
    void handleConnect(const net::Datagram &datagram, const Packet &packet, Time now);
    void handleSessionPacket(std::size_t slot, const Packet &packet, Time now);
    void handleDisconnect(std::size_t slot, const Packet &packet);
    bool acceptSequence(ClientSession &session, const Packet &packet);
    void confirm(std::size_t slot);
    void remove(std::size_t slot, std::uint8_t reason);
    void send(const net::Endpoint &peer, Packet packet);
    void broadcast(const Packet &packet);
    void reject(const net::Endpoint &peer, std::uint8_t reason);
    std::optional<std::size_t> findSession(const net::Endpoint &peer) const;
    std::optional<std::size_t> findFreeSlot() const;
    std::uint32_t newToken();
    static std::optional<std::uint32_t> tokenOf(const Packet &packet);

    NetworkSettings m_settings;
    Codec m_codec;
    net::UdpTransport m_transport;
    IGameSession &m_session;
    std::vector<std::optional<ClientSession>> m_clients;
    std::mt19937 m_random;
    std::uint32_t m_sequence;
    Time m_rateWindow;
    unsigned m_connectRequests;
};

} // namespace engine::network
