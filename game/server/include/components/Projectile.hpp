#pragma once

#include <cstddef>

namespace rtype::server {

// Marks a missile: it is removed once it leaves the world or hits a target.
// The owner slot is credited with the score of the enemies it destroys.
struct Projectile
{
    std::size_t ownerSlot = 0;
};

} // namespace rtype::server
