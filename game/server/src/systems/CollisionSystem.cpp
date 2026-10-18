#include "CollisionSystem.hpp"

#include <utility>
#include <vector>

#include "Collision.hpp"

namespace rtype::server {

CollisionSystem::CollisionSystem(ServerWorld &world, Listener onCollision)
    : m_world(world), m_onCollision(std::move(onCollision))
{
}

void CollisionSystem::update(float)
{
    std::vector<engine::physics::Body> bodies;
    for (std::size_t i : m_world.entities.with(POSITION | HITBOX)) {
        const Position &position = m_world.positions[i];
        const Hitbox &hitbox = m_world.hitboxes[i];
        bodies.push_back({i, {position.x, position.y, hitbox.width, hitbox.height}});
    }
    std::vector<std::pair<engine::ecs::Entity, engine::ecs::Entity>> collisions;
    for (const engine::physics::CollisionPair &pair : engine::physics::findCollisions(bodies))
        collisions.emplace_back(m_world.entities.entityAt(pair.first), m_world.entities.entityAt(pair.second));
    if (!m_onCollision)
        return;
    for (const auto &[first, second] : collisions)
        m_onCollision(first, second);
}

} // namespace rtype::server
