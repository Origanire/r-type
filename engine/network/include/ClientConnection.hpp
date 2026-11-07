#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Codec.hpp"
#include "Endpoint.hpp"
#include "NetworkSettings.hpp"
#include "Packet.hpp"
#include "ReliableReceiver.hpp"
#include "ReliableSender.hpp"
#include "SnapshotMessage.hpp"
#include "UdpTransport.hpp"

namespace engine::network {

class ClientConnection
{
public:
    ClientConnection(net::Endpoint server, const NetworkSettings &settings);

    void update(std::uint8_t buttons);
    void disconnect();
    bool connected() const;
    std::optional<SnapshotMessage> takeSnapshot();
    std::vector<Packet> takeEvents();

    void listRooms();
    void createRoom(const std::string &name, const std::string &code, const std::vector<std::uint8_t> &rules);
    void joinRoom(std::uint16_t roomId, const std::string &code);
    void leaveRoom();
    void startGame();
    void kickPlayer(std::uint8_t slot);
    void deleteRoom();

    std::uint64_t bytesReceived() const;
    std::uint64_t bytesSent() const;
    std::uint32_t pingMs() const;
    std::uint32_t roundTripMs() const;

private:
    using Time = std::chrono::steady_clock::time_point;

    bool handlePacket(const Packet &packet);
    bool handleSnapshot(const SnapshotMessage &snapshot);
    bool handleConnectionReply(const Packet &packet);
    void retryConnect(Time now);
    void sendPeriodic(std::uint8_t buttons, Time now);
    void send(Packet packet);
    static std::uint32_t nowMs(Time now);

    NetworkSettings m_settings;
    Codec m_codec;
    net::UdpTransport m_transport;
    net::Endpoint m_server;
    std::uint32_t m_sequence;
    ReliableSender m_sender;
    ReliableReceiver m_receiver;
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
    std::uint64_t m_bytesReceived;
    std::uint64_t m_bytesSent;
    std::uint32_t m_ping;
};

} // namespace engine::network
