#pragma once

#include <cstddef>
#include <vector>

#include "Enemy.hpp"
#include "EntityManager.hpp"
#include "Health.hpp"
#include "Hitbox.hpp"
#include "Invulnerable.hpp"
#include "PlayerInput.hpp"
#include "PlayerShip.hpp"
#include "Position.hpp"
#include "Projectile.hpp"
#include "Replicated.hpp"
#include "Shooter.hpp"
#include "Velocity.hpp"

namespace rtype::server {

inline constexpr std::size_t MAX_ENTITIES = 1024;

inline constexpr engine::ecs::ComponentMask POSITION = 1 << 0;
inline constexpr engine::ecs::ComponentMask VELOCITY = 1 << 1;
inline constexpr engine::ecs::ComponentMask HITBOX = 1 << 2;
inline constexpr engine::ecs::ComponentMask PLAYER_INPUT = 1 << 3;
inline constexpr engine::ecs::ComponentMask REPLICATED = 1 << 4;
inline constexpr engine::ecs::ComponentMask SHOOTER = 1 << 5;
inline constexpr engine::ecs::ComponentMask PROJECTILE = 1 << 6;
inline constexpr engine::ecs::ComponentMask ENEMY = 1 << 7;
inline constexpr engine::ecs::ComponentMask HEALTH = 1 << 8;
inline constexpr engine::ecs::ComponentMask PLAYER_SHIP = 1 << 9;
inline constexpr engine::ecs::ComponentMask INVULNERABLE = 1 << 10;

struct ServerWorld
{
    engine::ecs::EntityManager entities{MAX_ENTITIES};
    std::vector<Position> positions = std::vector<Position>(MAX_ENTITIES);
    std::vector<Velocity> velocities = std::vector<Velocity>(MAX_ENTITIES);
    std::vector<Hitbox> hitboxes = std::vector<Hitbox>(MAX_ENTITIES);
    std::vector<PlayerInput> inputs = std::vector<PlayerInput>(MAX_ENTITIES);
    std::vector<Replicated> replicated = std::vector<Replicated>(MAX_ENTITIES);
    std::vector<Shooter> shooters = std::vector<Shooter>(MAX_ENTITIES);
    std::vector<Projectile> projectiles = std::vector<Projectile>(MAX_ENTITIES);
    std::vector<Enemy> enemies = std::vector<Enemy>(MAX_ENTITIES);
    std::vector<Health> healths = std::vector<Health>(MAX_ENTITIES);
    std::vector<PlayerShip> playerShips = std::vector<PlayerShip>(MAX_ENTITIES);
    std::vector<Invulnerable> invulnerables = std::vector<Invulnerable>(MAX_ENTITIES);
};

} // namespace rtype::server
