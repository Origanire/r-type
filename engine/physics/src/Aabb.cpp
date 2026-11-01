#include "Aabb.hpp"

namespace engine::physics {

Aabb::Aabb(float left, float top, float width, float height)
    : m_left(left), m_top(top), m_width(width), m_height(height)
{
}

float Aabb::left() const
{
    return m_left;
}

float Aabb::top() const
{
    return m_top;
}

float Aabb::width() const
{
    return m_width;
}

float Aabb::height() const
{
    return m_height;
}

bool Aabb::overlaps(const Aabb &other) const
{
    return m_left < other.m_left + other.m_width && other.m_left < m_left + m_width
        && m_top < other.m_top + other.m_height && other.m_top < m_top + m_height;
}

} // namespace engine::physics
