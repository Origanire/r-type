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
inline constexpr float PLAYER_RESPAWN_DELAY = 2.f;
inline constexpr float PLAYER_INVULNERABILITY = 2.f;

inline constexpr float MISSILE_SPEED = 1500.f;
inline constexpr float MISSILE_WIDTH = 48.f;
inline constexpr float MISSILE_HEIGHT = 12.f;
inline constexpr float FIRE_INTERVAL = 0.2f;
inline constexpr int MISSILE_DAMAGE = 1;

inline constexpr float BYDO_SPEED = 250.f;
inline constexpr float BYDO_WIDTH = 66.f;
inline constexpr float BYDO_HEIGHT = 72.f;
inline constexpr float BYDO_SPAWN_MIN_INTERVAL = 0.6f;
inline constexpr float BYDO_SPAWN_MAX_INTERVAL = 1.8f;
inline constexpr float BYDO_SINE_AMPLITUDE = 150.f;
inline constexpr float BYDO_SINE_FREQUENCY = 3.f;
inline constexpr float BYDO_ZIGZAG_SPEED = 250.f;
inline constexpr int BYDO_HEALTH = 1;
inline constexpr std::uint32_t BYDO_SCORE = 100;

inline constexpr float GAME_OVER_DELAY = 5.f;

} // namespace rtype
