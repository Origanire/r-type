#pragma once

namespace engine::physics {

class Aabb
{
public:
    Aabb(float left, float top, float width, float height);

    float left() const;
    float top() const;
    float width() const;
    float height() const;
    bool overlaps(const Aabb &other) const;

private:
    float m_left;
    float m_top;
    float m_width;
    float m_height;
};

} // namespace engine::physics
