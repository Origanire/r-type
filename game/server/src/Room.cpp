#include "Room.hpp"

#include "BoundarySystem.hpp"
#include "Enemy.hpp"
#include "EnemyMovementSystem.hpp"
#include "EntityFactory.hpp"
#include "GameError.hpp"
#include "Hitbox.hpp"
#include "InputSystem.hpp"
#include "MovementSystem.hpp"
#include "OffscreenSystem.hpp"
#include "PlayerInput.hpp"
#include "Position.hpp"
#include "Projectile.hpp"
#include "Replicated.hpp"
#include "Shooter.hpp"
#include "ShootingSystem.hpp"
#include "SpawnSystem.hpp"
#include "SnapshotSystem.hpp"
#include "Velocity.hpp"

namespace rtype::server {

using engine::ecs::Phase;

Room::Room()
{
    m_registry.registerComponent<Position>();
    m_registry.registerComponent<Velocity>();
    m_registry.registerComponent<Hitbox>();
    m_registry.registerComponent<PlayerInput>();
    m_registry.registerComponent<Replicated>();
    m_registry.registerComponent<Shooter>();
    m_registry.registerComponent<Projectile>();
    m_registry.registerComponent<Enemy>();
    m_scheduler.addSystem(Phase::Input, inputSystem);
    m_scheduler.addSystem(Phase::Update, ShootingSystem([this] { return nextNetworkId(); }));
    m_scheduler.addSystem(Phase::Update, SpawnSystem([this] { return nextNetworkId(); }));
    m_scheduler.addSystem(Phase::Update, enemyMovementSystem);
    m_scheduler.addSystem(Phase::Physics, engine::physics::movementSystem);
    m_scheduler.addSystem(Phase::Physics, boundarySystem);
    m_scheduler.addSystem(Phase::Physics, offscreenSystem);
}

void Room::spawnPlayer(std::size_t slot)
{
    if (slot >= m_ships.size() || m_ships[slot])
        throw GameError("Room: invalid or occupied player slot");
    m_ships[slot] = createPlayerShip(m_registry, slot, nextNetworkId());
}

void Room::removePlayer(std::size_t slot)
{
    if (slot >= m_ships.size() || !m_ships[slot])
        return;
    m_registry.killEntity(*m_ships[slot]);
    m_ships[slot].reset();
}

void Room::setButtons(std::size_t slot, std::uint8_t buttons)
{
    if (slot >= m_ships.size() || !m_ships[slot])
        return;
    auto &input = m_registry.getComponents<PlayerInput>()[*m_ships[slot]];
    if (input)
        input->buttons = buttons;
}

void Room::update(float dt)
{
    m_scheduler.run(m_registry, dt);
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
    const auto &replicated = m_registry.getComponents<Replicated>()[*m_ships[slot]];
    return replicated ? replicated->networkId : 0;
}

std::vector<protocol::NetworkEntity> Room::networkEntities()
{
    return collectNetworkEntities(m_registry);
}

std::vector<protocol::Packet> Room::takeEvents()
{
    return m_events.take();
}

std::uint16_t Room::nextNetworkId()
{
    while (m_nextNetworkId == 0 || networkIdInUse(m_nextNetworkId))
        ++m_nextNetworkId;
    return m_nextNetworkId++;
}

bool Room::networkIdInUse(std::uint16_t networkId) const
{
    const auto &replicated = m_registry.getComponents<Replicated>();
    for (std::size_t i = 0; i < replicated.size(); ++i) {
        if (replicated[i] && replicated[i]->networkId == networkId)
            return true;
    }
    return false;
}

} // namespace rtype::server
