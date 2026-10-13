#pragma once

#include <array>
#include <chrono>
#include <optional>
#include <random>

#include "Registry.hpp"
#include "RtypeProtocol.hpp"
#include "UdpTransport.hpp"

namespace rtype::server {

class Server
{
public:
    explicit Server(std::uint16_t port);
    void step(std::chrono::steady_clock::time_point now);
    void shutdown();
private:
    using Time = std::chrono::steady_clock::time_point;
    struct Session
    {
        engine::net::Endpoint peer;
        std::uint32_t token;
        std::optional<engine::ecs::Entity> ship;
        Time lastSeen;
        Time lastInput;
        Time lastAccept;
        std::optional<std::uint32_t> inputSequence;
        std::optional<std::uint32_t> heartbeatSequence;
        std::uint8_t buttons = 0;
        bool confirmed = false;
    };
    void receive(const engine::net::Datagram &datagram, Time now);
    void send(const engine::net::Endpoint &peer, protocol::Packet packet);
    void remove(std::size_t slot, std::uint8_t reason);
    void snapshots();
    engine::net::UdpTransport m_transport;
    engine::ecs::Registry m_registry;
    std::array<std::optional<Session>, 4> m_sessions;
    std::mt19937 m_random;
    std::uint32_t m_sequence = 0;
    std::uint32_t m_tick = 0;
    std::uint16_t m_nextId = 1;
    std::array<std::uint16_t, 4> m_ids{};
    Time m_rateWindow{};
    unsigned m_connectRequests = 0;
};

} // namespace rtype::server
