#pragma once

#include "RenderTypes.hpp"

namespace rtype::client {

struct Star
{
    float speed = 0.f;
    float size = 1.f;
    engine::render::Color color;
};

} // namespace rtype::client
