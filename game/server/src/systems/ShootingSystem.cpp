#include "ShootingSystem.hpp"

#include <algorithm>
#include <utility>

#include "CommandBuffer.hpp"
#include "EntityFactory.hpp"
#include "GameConstants.hpp"
#include "PlayerInput.hpp"
#include "PlayerShip.hpp"
#include "Position.hpp"
#include "Registry.hpp"
#include "RtypeProtocol.hpp"
#include "Shooter.hpp"
#include "SnapshotSystem.hpp"

namespace rtype::server {

ShootingSystem::ShootingSystem(std::function<std::uint16_t()> nextNetworkId)
    : m_nextNetworkId(std::move(nextNetworkId))
{
}

void ShootingSystem::operator()(engine::ecs::SystemContext &context) const
{
    auto &inputs = context.registry.getComponents<PlayerInput>();
    auto &shooters = context.registry.getComponents<Shooter>();
    auto &positions = context.registry.getComponents<Position>();
    auto &ships = context.registry.getComponents<PlayerShip>();
    std::size_t replicatedCount = countReplicated(context.registry);
    for (std::size_t i = 0; i < inputs.size() && i < shooters.size() && i < positions.size(); ++i) {
        if (!inputs[i] || !shooters[i] || !positions[i] || i >= ships.size() || !ships[i])
            continue;
        shooters[i]->cooldown = std::max(0.f, shooters[i]->cooldown - context.dt);
        const bool fire = inputs[i]->buttons & protocol::BUTTON_FIRE;
        // Snapshots must stay complete: refuse the missile rather than exceed the entity limit.
        if (!fire || shooters[i]->cooldown > 0.f || replicatedCount >= protocol::MAX_ENTITIES)
            continue;
        shooters[i]->cooldown = FIRE_INTERVAL;
        ++replicatedCount;
        const Position origin{positions[i]->x + PLAYER_WIDTH, positions[i]->y + (PLAYER_HEIGHT - MISSILE_HEIGHT) / 2.f};
        const std::size_t ownerSlot = ships[i]->slot;
        const std::uint16_t networkId = m_nextNetworkId();
        context.commands.spawn(
            [origin, ownerSlot, networkId](engine::ecs::Registry &registry, const engine::ecs::Entity &missile) {
                createPlayerMissile(registry, missile, origin, ownerSlot, networkId);
            });
    }
}

} // namespace rtype::server
