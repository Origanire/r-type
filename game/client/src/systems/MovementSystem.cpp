#include "MovementSystem.hpp"

#include "Zipper.hpp"

namespace rtype::client {

void movementSystem(engine::ecs::SystemContext &context, engine::ecs::SparseArray<Position> &positions,
    engine::ecs::SparseArray<Velocity> &velocities)
{
    for (auto [position, velocity] : engine::ecs::Zipper(positions, velocities)) {
        position.x += velocity.x * context.dt;
        position.y += velocity.y * context.dt;
    }
}

} // namespace rtype::client
