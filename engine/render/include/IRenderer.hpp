#pragma once

#include <string>

#include "RenderTypes.hpp"

namespace engine::render {

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual void clear(Color color) = 0;
    virtual void drawRect(Vector2 position, Vector2 size, Color color) = 0;
    virtual void drawTexture(const std::string &texture, IntRect source, Vector2 position, Vector2 scale) = 0;
    virtual void display() = 0;
};

} // namespace engine::render
