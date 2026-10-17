#include "Aabb.hpp"

namespace engine::physics {

Aabb makeAabb(const Position &position, const Hitbox &hitbox)
{
    return {position.x, position.y, hitbox.width, hitbox.height};
}

bool overlaps(const Aabb &first, const Aabb &second)
{
    return first.left < second.left + second.width && second.left < first.left + first.width &&
        first.top < second.top + second.height && second.top < first.top + first.height;
}

} // namespace engine::physics
