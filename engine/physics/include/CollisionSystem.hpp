#pragma once

#include <functional>

#include "Entity.hpp"
#include "SystemContext.hpp"

namespace engine::physics {

// Two entities whose hitboxes overlap during this tick. Each pair is reported once, in index order.
struct Collision
{
    ecs::Entity first;
    ecs::Entity second;
};

// Detects overlapping hitboxes among the entities having a Position and a Hitbox.
// It only reports collisions: what they do (damage, destruction) is up to the listener.
class CollisionSystem
{
public:
    using Listener = std::function<void(const Collision &)>;

    explicit CollisionSystem(Listener onCollision);

    void operator()(ecs::SystemContext &context) const;

private:
    Listener m_onCollision;
};

} // namespace engine::physics
