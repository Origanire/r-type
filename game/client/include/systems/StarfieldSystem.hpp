#pragma once

#include "Position.hpp"
#include "SparseArray.hpp"
#include "Star.hpp"
#include "SystemContext.hpp"

namespace rtype::client {

void starfieldSystem(engine::ecs::SystemContext &context, engine::ecs::SparseArray<Position> &positions,
    engine::ecs::SparseArray<Star> &stars);

} // namespace rtype::client
