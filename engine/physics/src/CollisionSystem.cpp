#include "CollisionSystem.hpp"

#include <utility>
#include <vector>

#include "Aabb.hpp"
#include "PhysicsComponents.hpp"
#include "Registry.hpp"

namespace engine::physics {

CollisionSystem::CollisionSystem(Listener onCollision) : m_onCollision(std::move(onCollision))
{
}

void CollisionSystem::operator()(ecs::SystemContext &context) const
{
    auto &positions = context.registry.getComponents<Position>();
    auto &hitboxes = context.registry.getComponents<Hitbox>();
    std::vector<std::pair<std::size_t, Aabb>> bodies;
    for (std::size_t i = 0; i < positions.size() && i < hitboxes.size(); ++i) {
        if (positions[i] && hitboxes[i])
            bodies.emplace_back(i, makeAabb(*positions[i], *hitboxes[i]));
    }
    // A listener may kill entities, so every collision is found before the first one is reported.
    std::vector<Collision> collisions;
    for (std::size_t a = 0; a < bodies.size(); ++a) {
        for (std::size_t b = a + 1; b < bodies.size(); ++b) {
            if (overlaps(bodies[a].second, bodies[b].second))
                collisions.push_back({context.registry.entityFromIndex(bodies[a].first),
                    context.registry.entityFromIndex(bodies[b].first)});
        }
    }
    if (!m_onCollision)
        return;
    for (const Collision &collision : collisions)
        m_onCollision(collision);
}

} // namespace engine::physics
