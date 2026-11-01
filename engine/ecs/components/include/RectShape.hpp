#pragma once

#include "IComponent.hpp"
#include "Color.hpp"

namespace engine::ecs {

class RectShape : public IComponent
{
public:
    RectShape(float width, float height, Color color);

    float width() const;
    float height() const;
    Color color() const;

private:
    float m_width;
    float m_height;
    Color m_color;
};

} // namespace engine::ecs
