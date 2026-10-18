#include "ShipAnimationSystem.hpp"

#include <algorithm>
#include <cmath>

#include "SpriteSheet.hpp"

namespace rtype::client {

namespace {

constexpr float TILT_SPEED = 8.f;

} // namespace

ShipAnimationSystem::ShipAnimationSystem(ClientWorld &world) : m_world(world)
{
}

void ShipAnimationSystem::update(float dt)
{
    for (std::size_t i : m_world.entities.with(SHIP_TILT | SPRITE)) {
        ShipTilt &tilt = m_world.shipTilts[i];
        const float target = tilt.direction * PLAYER_SHIP_MAX_TILT;
        const float step = TILT_SPEED * dt;
        tilt.tilt += std::clamp(target - tilt.tilt, -step, step);
        const int column = PLAYER_SHIP_FLAT_COLUMN + static_cast<int>(std::lround(tilt.tilt));
        m_world.sprites[i].source = PLAYER_SHIP_SHEET.frame(column, tilt.row);
    }
}

} // namespace rtype::client
