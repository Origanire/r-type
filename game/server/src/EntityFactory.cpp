#include "EntityFactory.hpp"

#include "GameConstants.hpp"
#include "Hitbox.hpp"
#include "PlayerInput.hpp"
#include "Position.hpp"
#include "Projectile.hpp"
#include "Replicated.hpp"
#include "RtypeProtocol.hpp"
#include "Shooter.hpp"
#include "Velocity.hpp"

namespace rtype::server {

engine::ecs::Entity createPlayerShip(engine::ecs::Registry &registry, std::size_t slot, std::uint16_t networkId)
{
    const float spawnY = PLAYER_SPAWN_Y + static_cast<float>(slot) * PLAYER_SPAWN_SPACING;
    engine::ecs::Entity ship = registry.spawnEntity();
    registry.addComponent(ship, Position{PLAYER_SPAWN_X, spawnY});
    registry.addComponent(ship, Velocity{});
    registry.addComponent(ship, Hitbox{PLAYER_WIDTH, PLAYER_HEIGHT});
    registry.addComponent(ship, PlayerInput{});
    registry.addComponent(ship, Shooter{});
    registry.addComponent(ship, Replicated{networkId, protocol::KIND_PLAYER_SHIP});
    return ship;
}

void createPlayerMissile(
    engine::ecs::Registry &registry, const engine::ecs::Entity &missile, Position origin, std::uint16_t networkId)
{
    registry.addComponent(missile, origin);
    registry.addComponent(missile, Velocity{MISSILE_SPEED, 0.f});
    registry.addComponent(missile, Hitbox{MISSILE_WIDTH, MISSILE_HEIGHT});
    registry.addComponent(missile, Projectile{});
    registry.addComponent(missile, Replicated{networkId, protocol::KIND_PLAYER_MISSILE});
}

} // namespace rtype::server
