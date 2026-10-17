#pragma once

#include "SystemContext.hpp"

namespace engine::physics {

// Moves every entity having a Position and a Velocity by velocity * dt.
void movementSystem(ecs::SystemContext &context);

} // namespace engine::physics
