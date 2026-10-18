#pragma once

#include <cstdint>

namespace rtype::server {

enum class EnemyPattern : std::uint8_t
{
    Straight,
    Sine,
    Zigzag
};

inline constexpr int ENEMY_PATTERN_COUNT = 3;

struct Enemy
{
    EnemyPattern pattern = EnemyPattern::Straight;
    float baseY = 0.f;
    float age = 0.f;
};

} // namespace rtype::server
