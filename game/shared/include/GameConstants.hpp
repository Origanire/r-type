#pragma once

#include <cstdint>

namespace rtype {

inline constexpr float WORLD_WIDTH = 1920.f;
inline constexpr float WORLD_HEIGHT = 1080.f;

inline constexpr float PLAYER_SPEED = 600.f;
inline constexpr float PLAYER_WIDTH = 99.f;
inline constexpr float PLAYER_HEIGHT = 51.f;
inline constexpr float PLAYER_SPAWN_X = 200.f;
inline constexpr float PLAYER_SPAWN_Y = 200.f;
inline constexpr float PLAYER_SPAWN_SPACING = 160.f;
inline constexpr std::uint8_t PLAYER_START_LIVES = 3;

inline constexpr float MISSILE_SPEED = 1500.f;
inline constexpr float MISSILE_WIDTH = 48.f;
inline constexpr float MISSILE_HEIGHT = 12.f;
inline constexpr float FIRE_INTERVAL = 0.2f;

} // namespace rtype
