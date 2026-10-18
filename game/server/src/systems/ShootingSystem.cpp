#include "ShootingSystem.hpp"

#include <algorithm>
#include <utility>

#include "EntityFactory.hpp"
#include "GameConstants.hpp"
#include "RtypeProtocol.hpp"
#include "Snapshot.hpp"

namespace rtype::server {

ShootingSystem::ShootingSystem(ServerWorld &world, std::function<std::uint16_t()> nextNetworkId)
    : m_world(world), m_nextNetworkId(std::move(nextNetworkId))
{
}

void ShootingSystem::update(float dt)
{
    std::size_t replicatedCount = countReplicated(m_world);
    for (std::size_t i : m_world.entities.with(PLAYER_INPUT | SHOOTER | POSITION | PLAYER_SHIP)) {
        Shooter &shooter = m_world.shooters[i];
        shooter.cooldown = std::max(0.f, shooter.cooldown - dt);
        const bool fire = m_world.inputs[i].buttons & protocol::BUTTON_FIRE;
        if (!fire || shooter.cooldown > 0.f || replicatedCount >= protocol::MAX_ENTITIES)
            continue;
        shooter.cooldown = FIRE_INTERVAL;
        ++replicatedCount;
        const Position &position = m_world.positions[i];
        const Position origin{position.x + PLAYER_WIDTH, position.y + (PLAYER_HEIGHT - MISSILE_HEIGHT) / 2.f};
        createPlayerMissile(m_world, origin, m_world.playerShips[i].slot, m_nextNetworkId());
    }
}

} // namespace rtype::server
