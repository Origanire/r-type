#include "BoundarySystem.hpp"

#include <algorithm>

#include "GameConstants.hpp"

namespace rtype::server {

BoundarySystem::BoundarySystem(ServerWorld &world) : m_world(world)
{
}

void BoundarySystem::update(float)
{
    for (std::size_t i : m_world.entities.with(POSITION | HITBOX | PLAYER_INPUT)) {
        Position &position = m_world.positions[i];
        const Hitbox &hitbox = m_world.hitboxes[i];
        position.x = std::clamp(position.x, 0.f, WORLD_WIDTH - hitbox.width);
        position.y = std::clamp(position.y, 0.f, WORLD_HEIGHT - hitbox.height);
    }
}

} // namespace rtype::server
