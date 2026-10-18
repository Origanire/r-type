#pragma once

#include "RenderTypes.hpp"

namespace rtype::client {

struct SpriteSheet
{
    const char *texture;
    const char *path;
    int left;
    int top;
    int frameWidth;
    int frameHeight;

    engine::render::IntRect frame(int column, int row) const;
};

inline constexpr SpriteSheet PLAYER_SHIP_SHEET{"player_ships", "players/r-typesheet42.gif", 0, 0, 33, 17};
inline constexpr int PLAYER_SHIP_FLAT_COLUMN = 2;
inline constexpr int PLAYER_SHIP_MAX_TILT = 2;

inline constexpr SpriteSheet PLAYER_MISSILE_SHEET{"player_missiles", "projectiles/r-typesheet1.gif", 249, 90, 16, 4};

inline constexpr SpriteSheet BYDO_SHEET{"bydos", "enemies/r-typesheet5.gif", 0, 0, 33, 36};

} // namespace rtype::client
