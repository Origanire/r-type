#pragma once

namespace rtype::server {

// A freshly respawned ship ignores collisions until the timer runs out.
struct Invulnerable
{
    float remaining = 0.f;
};

} // namespace rtype::server
