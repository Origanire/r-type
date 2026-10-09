#pragma once

#include <string>

#include "RenderTypes.hpp"

namespace rtype::client {

struct Sprite
{
    std::string texture;
    engine::render::IntRect source;
    float scale = 1.f;
};

} // namespace rtype::client
