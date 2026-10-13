#include "EntityFactory.hpp"

#include "GameConstants.hpp"
#include "Hitbox.hpp"
#include "PlayerInput.hpp"
#include "Position.hpp"
#include "Replicated.hpp"
#include "RtypeProtocol.hpp"
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
    registry.addComponent(ship, Replicated{networkId, protocol::KIND_PLAYER_SHIP});
    return ship;
}

} // namespace rtype::server
