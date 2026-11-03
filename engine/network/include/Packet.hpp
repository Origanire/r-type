#pragma once

#include <cstdint>
#include <variant>

#include "AcceptMessage.hpp"
#include "ConnectMessage.hpp"
#include "DisconnectMessage.hpp"
#include "EntityDestroyedMessage.hpp"
#include "HeartbeatAckMessage.hpp"
#include "HeartbeatMessage.hpp"
#include "InputMessage.hpp"
#include "PlayerJoinedMessage.hpp"
#include "PlayerKilledMessage.hpp"
#include "PlayerLeftMessage.hpp"
#include "RejectMessage.hpp"
#include "Protocol.hpp"
#include "ServerShutdownMessage.hpp"
#include "SnapshotMessage.hpp"

namespace engine::network {

class Packet
{
public:
    using Body = std::variant<ConnectMessage, InputMessage, HeartbeatMessage, DisconnectMessage, AcceptMessage,
        RejectMessage, SnapshotMessage, HeartbeatAckMessage, EntityDestroyedMessage, PlayerJoinedMessage,
        PlayerKilledMessage, PlayerLeftMessage, ServerShutdownMessage>;

    template <class Message>
    explicit Packet(Message message, std::uint32_t sequence = 0, std::uint8_t version = Protocol::VERSION);

    MessageType type() const;
    std::uint8_t version() const;
    std::uint32_t sequence() const;
    void setSequence(std::uint32_t sequence);

    template <class Message>
    bool holds() const;
    template <class Message>
    const Message &as() const;

private:
    MessageType m_type;
    std::uint8_t m_version;
    std::uint32_t m_sequence;
    Body m_body;
};

} // namespace engine::network

#include "Packet.tpp"
