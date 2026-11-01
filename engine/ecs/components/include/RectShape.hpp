#pragma once

#include "IComponent.hpp"
#include "RenderTypes.hpp"

namespace engine::ecs {

class RectShape : public IComponent
{
public:
    RectShape(float width, float height, render::Color color);

    float width() const;
    float height() const;
    render::Color color() const;

private:
    float m_width;
    float m_height;
    render::Color m_color;
};

} // namespace engine::ecs
