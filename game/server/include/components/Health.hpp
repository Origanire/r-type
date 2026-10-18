#pragma once

namespace rtype::server {

// Hit points of a destructible entity: it is destroyed once they reach zero.
struct Health
{
    int points = 0;
};

} // namespace rtype::server
