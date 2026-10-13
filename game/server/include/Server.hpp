#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>

#include "Room.hpp"
#include "RtypeProtocol.hpp"
#include "UdpTransport.hpp"

namespace rtype::server {

class Server
{
public:
    explicit Server(std::uint16_t port);

    void poll();
    void tick(float dt);
    void shutdown();

private:
    using Time = std::chrono::steady_clock::time_point;

    struct Session
    {
        engine::net::Endpoint peer;
        std::uint32_t token = 0;
        Time lastSeen;
        Time lastInput;
        Time lastAccept;
        std::optional<std::uint32_t> inputSequence;
        std::optional<std::uint32_t> heartbeatSequence;
        bool confirmed = false;
    };

    void receive(const engine::net::Datagram &datagram, Time now);
    void handleConnect(const engine::net::Datagram &datagram, const protocol::Packet &packet, Time now);
    void handleSessionPacket(std::size_t slot, const protocol::Packet &packet, Time now);
    void confirm(std::size_t slot);
    void remove(std::size_t slot, std::uint8_t reason);
    void sendSnapshots();
    void send(const engine::net::Endpoint &peer, protocol::Packet packet);
    void broadcast(const protocol::Packet &packet);
    void reject(const engine::net::Endpoint &peer, std::uint8_t reason);
    std::optional<std::size_t> findSession(const engine::net::Endpoint &peer) const;
    std::optional<std::size_t> findFreeSlot() const;
    std::uint32_t newToken();

    engine::net::UdpTransport m_transport;
    Room m_room;
    std::array<std::optional<Session>, protocol::MAX_PLAYERS> m_sessions;
    std::mt19937 m_random;
    std::uint32_t m_sequence = 0;
    Time m_rateWindow{};
    unsigned m_connectRequests = 0;
};

} // namespace rtype::server
