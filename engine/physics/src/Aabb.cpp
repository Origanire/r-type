#include "Aabb.hpp"

namespace engine::physics {

bool overlaps(const Aabb &first, const Aabb &second)
{
    return first.left < second.left + second.width && second.left < first.left + first.width &&
        first.top < second.top + second.height && second.top < first.top + first.height;
}

} // namespace engine::physics
