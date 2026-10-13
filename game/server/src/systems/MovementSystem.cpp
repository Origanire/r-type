#include "MovementSystem.hpp"

#include "Position.hpp"
#include "Registry.hpp"
#include "Velocity.hpp"

namespace rtype::server {

void movementSystem(engine::ecs::SystemContext &context)
{
    auto &positions = context.registry.getComponents<Position>();
    auto &velocities = context.registry.getComponents<Velocity>();
    for (std::size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        if (!positions[i] || !velocities[i])
            continue;
        positions[i]->x += velocities[i]->x * context.dt;
        positions[i]->y += velocities[i]->y * context.dt;
    }
}

} // namespace rtype::server
