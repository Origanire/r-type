#pragma once

namespace engine::physics {

struct Aabb
{
    float left = 0.f;
    float top = 0.f;
    float width = 0.f;
    float height = 0.f;
};

bool overlaps(const Aabb &first, const Aabb &second);

} // namespace engine::physics
