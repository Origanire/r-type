#pragma once

#include <cstddef>

namespace rtype::server {

// Links a ship to the player slot controlling it.
struct PlayerShip
{
    std::size_t slot = 0;
};

} // namespace rtype::server
