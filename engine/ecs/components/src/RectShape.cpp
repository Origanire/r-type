#include "RectShape.hpp"

namespace engine::ecs {

RectShape::RectShape(float width, float height, Color color) : m_width(width), m_height(height), m_color(color)
{
}

float RectShape::width() const
{
    return m_width;
}

float RectShape::height() const
{
    return m_height;
}

Color RectShape::color() const
{
    return m_color;
}

} // namespace engine::ecs
