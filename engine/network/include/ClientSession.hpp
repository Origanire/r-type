#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

#include "Endpoint.hpp"

namespace engine::network {

class ClientSession
{
public:
    using Time = std::chrono::steady_clock::time_point;

    ClientSession(net::Endpoint peer, std::uint32_t token, Time now);

    const net::Endpoint &peer() const;
    std::uint32_t token() const;
    bool confirmed() const;
    void confirm();

    Time lastSeen() const;
    void markSeen(Time now);
    Time lastInput() const;
    void markInput(Time now);
    Time lastAccept() const;
    void markAccepted(Time now);

    std::optional<std::uint32_t> inputSequence() const;
    void setInputSequence(std::uint32_t sequence);
    std::optional<std::uint32_t> heartbeatSequence() const;
    void setHeartbeatSequence(std::uint32_t sequence);

private:
    net::Endpoint m_peer;
    std::uint32_t m_token;
    bool m_confirmed;
    Time m_lastSeen;
    Time m_lastInput;
    Time m_lastAccept;
    std::optional<std::uint32_t> m_inputSequence;
    std::optional<std::uint32_t> m_heartbeatSequence;
};

} // namespace engine::network
