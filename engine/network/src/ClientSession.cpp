#include "ClientSession.hpp"

#include <utility>

namespace engine::network {

ClientSession::ClientSession(net::Endpoint peer, std::uint32_t token, Time now)
    : m_peer(std::move(peer)), m_token(token), m_confirmed(false), m_lastSeen(now), m_lastInput(now), m_lastAccept(),
      m_inputSequence(), m_heartbeatSequence()
{
}

const net::Endpoint &ClientSession::peer() const
{
    return m_peer;
}

std::uint32_t ClientSession::token() const
{
    return m_token;
}

bool ClientSession::confirmed() const
{
    return m_confirmed;
}

void ClientSession::confirm()
{
    m_confirmed = true;
}

ClientSession::Time ClientSession::lastSeen() const
{
    return m_lastSeen;
}

void ClientSession::markSeen(Time now)
{
    m_lastSeen = now;
}

ClientSession::Time ClientSession::lastInput() const
{
    return m_lastInput;
}

void ClientSession::markInput(Time now)
{
    m_lastInput = now;
}

ClientSession::Time ClientSession::lastAccept() const
{
    return m_lastAccept;
}

void ClientSession::markAccepted(Time now)
{
    m_lastAccept = now;
}

std::optional<std::uint32_t> ClientSession::inputSequence() const
{
    return m_inputSequence;
}

void ClientSession::setInputSequence(std::uint32_t sequence)
{
    m_inputSequence = sequence;
}

std::optional<std::uint32_t> ClientSession::heartbeatSequence() const
{
    return m_heartbeatSequence;
}

void ClientSession::setHeartbeatSequence(std::uint32_t sequence)
{
    m_heartbeatSequence = sequence;
}

} // namespace engine::network
