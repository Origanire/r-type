#pragma once

#include <cstddef>
#include <vector>

#include "EntityManager.hpp"
#include "Hitbox.hpp"
#include "PlayerInput.hpp"
#include "Position.hpp"
#include "Replicated.hpp"
#include "Velocity.hpp"

namespace rtype::server {

inline constexpr std::size_t MAX_ENTITIES = 1024;

inline constexpr engine::ecs::ComponentMask POSITION = 1 << 0;
inline constexpr engine::ecs::ComponentMask VELOCITY = 1 << 1;
inline constexpr engine::ecs::ComponentMask HITBOX = 1 << 2;
inline constexpr engine::ecs::ComponentMask PLAYER_INPUT = 1 << 3;
inline constexpr engine::ecs::ComponentMask REPLICATED = 1 << 4;

struct ServerWorld
{
    engine::ecs::EntityManager entities{MAX_ENTITIES};
    std::vector<Position> positions = std::vector<Position>(MAX_ENTITIES);
    std::vector<Velocity> velocities = std::vector<Velocity>(MAX_ENTITIES);
    std::vector<Hitbox> hitboxes = std::vector<Hitbox>(MAX_ENTITIES);
    std::vector<PlayerInput> inputs = std::vector<PlayerInput>(MAX_ENTITIES);
    std::vector<Replicated> replicated = std::vector<Replicated>(MAX_ENTITIES);
};

} // namespace rtype::server
