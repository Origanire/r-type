#include "ScreenClampSystem.hpp"

#include <algorithm>

#include "WorldSize.hpp"
#include "Zipper.hpp"

namespace rtype::client {

void screenClampSystem(engine::ecs::SystemContext &, engine::ecs::SparseArray<Position> &positions,
    engine::ecs::SparseArray<Sprite> &sprites, engine::ecs::SparseArray<PlayerControl> &controls)
{
    for (auto [position, sprite, control] : engine::ecs::Zipper(positions, sprites, controls)) {
        float width = static_cast<float>(sprite.source.width) * sprite.scale;
        float height = static_cast<float>(sprite.source.height) * sprite.scale;
        position.x = std::clamp(position.x, 0.f, WORLD_WIDTH - width);
        position.y = std::clamp(position.y, 0.f, WORLD_HEIGHT - height);
    }
}

} // namespace rtype::client
