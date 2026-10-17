#pragma once

#include "PhysicsComponents.hpp"

namespace engine::physics {

// Axis-aligned bounding box: (left, top) is its top-left corner.
struct Aabb
{
    float left = 0.f;
    float top = 0.f;
    float width = 0.f;
    float height = 0.f;
};

Aabb makeAabb(const Position &position, const Hitbox &hitbox);
// Boxes that only touch by an edge do not overlap.
bool overlaps(const Aabb &first, const Aabb &second);

} // namespace engine::physics
