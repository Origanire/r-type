#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>

#include "Codec.hpp"
#include "Endpoint.hpp"
#include "NetworkSettings.hpp"
#include "Packet.hpp"
#include "SnapshotMessage.hpp"
#include "UdpTransport.hpp"

namespace engine::network {

class ClientConnection
{
public:
    ClientConnection(net::Endpoint server, const NetworkSettings &settings);

    void update(std::uint8_t buttons);
    void disconnect();
    std::optional<SnapshotMessage> takeSnapshot();
    std::vector<Packet> takeEvents();

private:
    using Time = std::chrono::steady_clock::time_point;

    bool connected() const;
    bool handlePacket(const Packet &packet);
    bool handleSnapshot(const SnapshotMessage &snapshot);
    bool handleConnectionReply(const Packet &packet);
    void retryConnect(Time now);
    void sendPeriodic(std::uint8_t buttons, Time now);
    void send(Packet packet);

    NetworkSettings m_settings;
    Codec m_codec;
    net::UdpTransport m_transport;
    net::Endpoint m_server;
    std::uint32_t m_sequence;
    std::uint32_t m_token;
    unsigned m_attempts;
    Time m_lastConnect;
    Time m_lastInput;
    Time m_lastHeartbeat;
    Time m_lastReceive;
    std::optional<std::uint32_t> m_lastTick;
    std::optional<std::uint32_t> m_serverSequence;
    std::optional<SnapshotMessage> m_snapshot;
    std::vector<Packet> m_events;
};

} // namespace engine::network
