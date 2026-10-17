#include "GameEvents.hpp"

#include <utility>

namespace rtype::server {

void GameEvents::entityDestroyed(std::uint16_t networkId)
{
    protocol::Packet event;
    event.type = protocol::MessageType::EntityDestroyed;
    event.entityId = networkId;
    m_events.push_back(event);
}

void GameEvents::playerKilled(std::size_t slot)
{
    protocol::Packet event;
    event.type = protocol::MessageType::PlayerKilled;
    event.playerId = static_cast<std::uint8_t>(slot);
    m_events.push_back(event);
}

std::vector<protocol::Packet> GameEvents::take()
{
    std::vector<protocol::Packet> events = std::move(m_events);
    m_events.clear();
    return events;
}

} // namespace rtype::server
