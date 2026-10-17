#pragma once

#include "RenderTypes.hpp"

namespace rtype::client {

// Part of a sprite sheet cut into a grid of same-sized frames, starting at (left, top).
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

// r-typesheet42: one row per ship color, one column per tilt.
// Columns 0-1 lean down, column 2 is flat, columns 3-4 lean up.
inline constexpr SpriteSheet PLAYER_SHIP_SHEET{"player_ships", "players/r-typesheet42.gif", 0, 0, 33, 17};
inline constexpr int PLAYER_SHIP_FLAT_COLUMN = 2;
inline constexpr int PLAYER_SHIP_MAX_TILT = 2;

// r-typesheet1: the basic shot of the player ship, its trail on the left.
inline constexpr SpriteSheet PLAYER_MISSILE_SHEET{"player_missiles", "projectiles/r-typesheet1.gif", 249, 90, 16, 4};

// r-typesheet5: the basic Bydo, facing left, one column per animation frame.
inline constexpr SpriteSheet BYDO_SHEET{"bydos", "enemies/r-typesheet5.gif", 0, 0, 33, 36};

} // namespace rtype::client
