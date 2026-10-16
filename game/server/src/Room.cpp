#include "Room.hpp"

#include <memory>

#include "BoundarySystem.hpp"
#include "EntityFactory.hpp"
#include "GameError.hpp"
#include "InputSystem.hpp"
#include "MovementSystem.hpp"
#include "Snapshot.hpp"

namespace rtype::server {

Room::Room()
{
    m_systems.add(std::make_unique<InputSystem>(m_world));
    m_systems.add(std::make_unique<MovementSystem>(m_world));
    m_systems.add(std::make_unique<BoundarySystem>(m_world));
}

void Room::spawnPlayer(std::size_t slot)
{
    if (slot >= m_ships.size() || m_ships[slot])
        throw GameError("Room: invalid or occupied player slot");
    m_ships[slot] = createPlayerShip(m_world, slot, nextNetworkId());
}

void Room::removePlayer(std::size_t slot)
{
    if (slot >= m_ships.size() || !m_ships[slot])
        return;
    m_world.entities.destroy(*m_ships[slot]);
    m_ships[slot].reset();
}

void Room::setButtons(std::size_t slot, std::uint8_t buttons)
{
    if (slot >= m_ships.size() || !m_ships[slot])
        return;
    m_world.inputs[m_ships[slot]->index()].buttons = buttons;
}

void Room::update(float dt)
{
    m_systems.update(dt);
    ++m_tick;
}

std::uint32_t Room::tick() const
{
    return m_tick;
}

std::uint16_t Room::playerNetworkId(std::size_t slot) const
{
    if (slot >= m_ships.size() || !m_ships[slot])
        return 0;
    return m_world.replicated[m_ships[slot]->index()].networkId;
}

std::vector<protocol::NetworkEntity> Room::networkEntities() const
{
    return collectNetworkEntities(m_world);
}

std::uint16_t Room::nextNetworkId()
{
    while (m_nextNetworkId == 0 || networkIdInUse(m_nextNetworkId))
        ++m_nextNetworkId;
    return m_nextNetworkId++;
}

bool Room::networkIdInUse(std::uint16_t networkId) const
{
    for (std::size_t i : m_world.entities.with(REPLICATED)) {
        if (m_world.replicated[i].networkId == networkId)
            return true;
    }
    return false;
}

} // namespace rtype::server
