#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>

#include "RtypeProtocol.hpp"
#include "UdpTransport.hpp"

namespace rtype::client {

class NetworkClient
{
public:
    explicit NetworkClient(engine::net::Endpoint server);

    void update(std::uint8_t buttons);
    void disconnect();
    std::optional<protocol::WorldSnapshot> takeSnapshot();
    std::vector<protocol::Packet> takeEvents();

private:
    using Time = std::chrono::steady_clock::time_point;

    bool connected() const;
    bool handlePacket(const protocol::Packet &packet);
    bool handleConnectionReply(const protocol::Packet &packet);
    void retryConnect(Time now);
    void sendPeriodic(std::uint8_t buttons, Time now);
    void send(protocol::Packet packet);

    engine::net::UdpTransport m_transport;
    engine::net::Endpoint m_server;
    std::uint32_t m_sequence = 0;
    std::uint32_t m_token = 0;
    unsigned m_attempts = 0;
    Time m_lastConnect{};
    Time m_lastInput{};
    Time m_lastHeartbeat{};
    Time m_lastReceive{};
    std::optional<std::uint32_t> m_lastTick;
    std::optional<std::uint32_t> m_serverSequence;
    std::optional<protocol::WorldSnapshot> m_snapshot;
    std::vector<protocol::Packet> m_events;
};

} // namespace rtype::client
