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
    world.shooters[i] = Shooter{};
    world.playerShips[i] = PlayerShip{slot};
    world.replicated[i] = Replicated{networkId, protocol::KIND_PLAYER_SHIP};
    world.entities.addComponents(
        ship, POSITION | VELOCITY | HITBOX | PLAYER_INPUT | SHOOTER | PLAYER_SHIP | REPLICATED);
    return ship;
}

engine::ecs::Entity createPlayerMissile(
    ServerWorld &world, Position origin, std::size_t ownerSlot, std::uint16_t networkId)
{
    engine::ecs::Entity missile = world.entities.create();
    std::size_t i = missile.index();
    world.positions[i] = origin;
    world.velocities[i] = Velocity{MISSILE_SPEED, 0.f};
    world.hitboxes[i] = Hitbox{MISSILE_WIDTH, MISSILE_HEIGHT};
    world.projectiles[i] = Projectile{ownerSlot};
    world.replicated[i] = Replicated{networkId, protocol::KIND_PLAYER_MISSILE};
    world.entities.addComponents(missile, POSITION | VELOCITY | HITBOX | PROJECTILE | REPLICATED);
    return missile;
}

engine::ecs::Entity createBydo(
    ServerWorld &world, Position origin, EnemyPattern pattern, bool goingUp, std::uint16_t networkId)
{
    float verticalSpeed = 0.f;
    if (pattern == EnemyPattern::Zigzag)
        verticalSpeed = goingUp ? -BYDO_ZIGZAG_SPEED : BYDO_ZIGZAG_SPEED;
    engine::ecs::Entity bydo = world.entities.create();
    std::size_t i = bydo.index();
    world.positions[i] = origin;
    world.velocities[i] = Velocity{-BYDO_SPEED, verticalSpeed};
    world.hitboxes[i] = Hitbox{BYDO_WIDTH, BYDO_HEIGHT};
    world.enemies[i] = Enemy{pattern, origin.y, 0.f};
    world.healths[i] = Health{BYDO_HEALTH};
    world.replicated[i] = Replicated{networkId, protocol::KIND_BYDO};
    world.entities.addComponents(bydo, POSITION | VELOCITY | HITBOX | ENEMY | HEALTH | REPLICATED);
    return bydo;
}

} // namespace rtype::server
