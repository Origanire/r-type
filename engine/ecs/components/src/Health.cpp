#include "Health.hpp"

namespace engine::ecs {

Health::Health(int points) : m_points(points)
{
}

int Health::points() const
{
    return m_points;
}

void Health::damage(int amount)
{
    m_points -= amount;
}

bool Health::isDead() const
{
    return m_points <= 0;
}

} // namespace engine::ecs
