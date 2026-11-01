#include "MovementPattern.hpp"

namespace engine::ecs {

MovementPattern::MovementPattern(
    Kind kind, float amplitude, float frequency, float verticalSpeed, float minY, float maxY)
    : m_kind(kind), m_amplitude(amplitude), m_frequency(frequency), m_verticalSpeed(verticalSpeed), m_minY(minY),
      m_maxY(maxY), m_age(0.f), m_baseY(0.f), m_hasBaseY(false)
{
}

MovementPattern::Kind MovementPattern::kind() const
{
    return m_kind;
}

float MovementPattern::amplitude() const
{
    return m_amplitude;
}

float MovementPattern::frequency() const
{
    return m_frequency;
}

float MovementPattern::verticalSpeed() const
{
    return m_verticalSpeed;
}

float MovementPattern::minY() const
{
    return m_minY;
}

float MovementPattern::maxY() const
{
    return m_maxY;
}

float MovementPattern::age() const
{
    return m_age;
}

void MovementPattern::advance(float dt)
{
    m_age += dt;
}

bool MovementPattern::hasBaseY() const
{
    return m_hasBaseY;
}

float MovementPattern::baseY() const
{
    return m_baseY;
}

void MovementPattern::setBaseY(float y)
{
    m_baseY = y;
    m_hasBaseY = true;
}

} // namespace engine::ecs
