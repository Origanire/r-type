#pragma once

#include "PlayerControl.hpp"
#include "Position.hpp"
#include "SparseArray.hpp"
#include "Sprite.hpp"
#include "SystemContext.hpp"

namespace rtype::client {

void screenClampSystem(engine::ecs::SystemContext &context, engine::ecs::SparseArray<Position> &positions,
    engine::ecs::SparseArray<Sprite> &sprites, engine::ecs::SparseArray<PlayerControl> &controls);

} // namespace rtype::client
