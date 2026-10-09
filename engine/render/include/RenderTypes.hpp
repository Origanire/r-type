#pragma once

#include <cstdint>

namespace engine::render {

struct Color
{
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;
};

struct Vector2
{
    float x = 0.f;
    float y = 0.f;
};

struct IntRect
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

} // namespace engine::render
