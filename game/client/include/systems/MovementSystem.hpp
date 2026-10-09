#pragma once

#include "Position.hpp"
#include "SparseArray.hpp"
#include "SystemContext.hpp"
#include "Velocity.hpp"

namespace rtype::client {

void movementSystem(engine::ecs::SystemContext &context, engine::ecs::SparseArray<Position> &positions,
    engine::ecs::SparseArray<Velocity> &velocities);

} // namespace rtype::client
