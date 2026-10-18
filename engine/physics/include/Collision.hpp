#pragma once

#include <cstddef>
#include <vector>

#include "Aabb.hpp"

namespace engine::physics {

struct Body
{
    std::size_t index = 0;
    Aabb box;
};

struct CollisionPair
{
    std::size_t first = 0;
    std::size_t second = 0;
};

std::vector<CollisionPair> findCollisions(const std::vector<Body> &bodies);

} // namespace engine::physics
