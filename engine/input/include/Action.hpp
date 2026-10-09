#pragma once

#include <cstddef>
#include <cstdint>

namespace engine::input {

enum class Action : std::uint8_t
{
    Up,
    Down,
    Left,
    Right,
    Fire
};

inline constexpr std::size_t ACTION_COUNT = static_cast<std::size_t>(Action::Fire) + 1;

} // namespace engine::input
