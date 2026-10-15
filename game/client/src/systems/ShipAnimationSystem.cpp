#include "ShipAnimationSystem.hpp"

#include <algorithm>
#include <cmath>

#include "Registry.hpp"
#include "ShipTilt.hpp"
#include "Sprite.hpp"
#include "SpriteSheet.hpp"

namespace rtype::client {

namespace {

// Frames per second: a ship reaches its full tilt in a quarter of a second.
constexpr float TILT_SPEED = 8.f;

} // namespace

void shipAnimationSystem(engine::ecs::SystemContext &context)
{
    auto &tilts = context.registry.getComponents<ShipTilt>();
    auto &sprites = context.registry.getComponents<Sprite>();
    for (std::size_t i = 0; i < tilts.size() && i < sprites.size(); ++i) {
        if (!tilts[i] || !sprites[i])
            continue;
        const float target = tilts[i]->direction * PLAYER_SHIP_MAX_TILT;
        const float step = TILT_SPEED * context.dt;
        tilts[i]->tilt += std::clamp(target - tilts[i]->tilt, -step, step);
        const int column = PLAYER_SHIP_FLAT_COLUMN + static_cast<int>(std::lround(tilts[i]->tilt));
        sprites[i]->source = PLAYER_SHIP_SHEET.frame(column, tilts[i]->row);
    }
}

} // namespace rtype::client
