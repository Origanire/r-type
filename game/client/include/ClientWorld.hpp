#pragma once

#include <cstddef>
#include <vector>

#include "EntityManager.hpp"
#include "Position.hpp"
#include "Sprite.hpp"
#include "Star.hpp"

namespace rtype::client {

inline constexpr std::size_t MAX_ENTITIES = 1024;

inline constexpr engine::ecs::ComponentMask POSITION = 1 << 0;
inline constexpr engine::ecs::ComponentMask SPRITE = 1 << 1;
inline constexpr engine::ecs::ComponentMask STAR = 1 << 2;

struct ClientWorld
{
    engine::ecs::EntityManager entities{MAX_ENTITIES};
    std::vector<Position> positions = std::vector<Position>(MAX_ENTITIES);
    std::vector<Sprite> sprites = std::vector<Sprite>(MAX_ENTITIES);
    std::vector<Star> stars = std::vector<Star>(MAX_ENTITIES);
};

} // namespace rtype::client
