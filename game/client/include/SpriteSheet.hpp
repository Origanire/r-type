#pragma once

#include "RenderTypes.hpp"

namespace rtype::client {

// A sprite sheet cut into a grid of same-sized frames, read left to right and top to bottom.
struct SpriteSheet
{
    const char *texture;
    const char *path;
    int frameWidth;
    int frameHeight;

    engine::render::IntRect frame(int column, int row) const;
};

// r-typesheet42: one row per ship color, one column per tilt.
// Columns 0-1 lean down, column 2 is flat, columns 3-4 lean up.
inline constexpr SpriteSheet PLAYER_SHIP_SHEET{"player_ships", "players/r-typesheet42.gif", 33, 17};
inline constexpr int PLAYER_SHIP_FLAT_COLUMN = 2;
inline constexpr int PLAYER_SHIP_MAX_TILT = 2;

} // namespace rtype::client
