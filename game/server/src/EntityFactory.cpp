#include "EntityFactory.hpp"

#include "GameConstants.hpp"
#include "RtypeProtocol.hpp"

namespace rtype::server {

engine::ecs::Entity createPlayerShip(ServerWorld &world, std::size_t slot, std::uint16_t networkId)
{
    const float spawnY = PLAYER_SPAWN_Y + static_cast<float>(slot) * PLAYER_SPAWN_SPACING;
    engine::ecs::Entity ship = world.entities.create();
    std::size_t i = ship.index();
    world.positions[i] = Position{PLAYER_SPAWN_X, spawnY};
    world.velocities[i] = Velocity{};
    world.hitboxes[i] = Hitbox{PLAYER_WIDTH, PLAYER_HEIGHT};
    world.inputs[i] = PlayerInput{};
    world.replicated[i] = Replicated{networkId, protocol::KIND_PLAYER_SHIP};
    world.entities.addComponents(ship, POSITION | VELOCITY | HITBOX | PLAYER_INPUT | REPLICATED);
    return ship;
}

} // namespace rtype::server
