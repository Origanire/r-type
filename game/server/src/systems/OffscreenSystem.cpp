#include "OffscreenSystem.hpp"

#include "GameConstants.hpp"

namespace rtype::server {

OffscreenSystem::OffscreenSystem(ServerWorld &world) : m_world(world)
{
}

void OffscreenSystem::update(float)
{
    for (std::size_t i : m_world.entities.with(POSITION | HITBOX | PROJECTILE)) {
        const Position &position = m_world.positions[i];
        const Hitbox &hitbox = m_world.hitboxes[i];
        const bool outside = position.x + hitbox.width <= 0.f || position.x >= WORLD_WIDTH ||
            position.y >= WORLD_HEIGHT || position.y + hitbox.height <= 0.f;
        if (outside)
            m_world.entities.destroy(m_world.entities.entityAt(i));
    }
    for (std::size_t i : m_world.entities.with(POSITION | HITBOX | ENEMY)) {
        if (m_world.positions[i].x + m_world.hitboxes[i].width <= 0.f)
            m_world.entities.destroy(m_world.entities.entityAt(i));
    }
}

} // namespace rtype::server
