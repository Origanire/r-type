#include "AutoFire.hpp"

#include <utility>

namespace engine::ecs {

AutoFire::AutoFire(std::string prefab, float minInterval, float maxInterval, float offsetX, float offsetY)
    : m_prefab(std::move(prefab)), m_minInterval(minInterval), m_maxInterval(maxInterval), m_offsetX(offsetX),
      m_offsetY(offsetY), m_timer(0.f), m_started(false)
{
}

const std::string &AutoFire::prefab() const
{
    return m_prefab;
}

float AutoFire::minInterval() const
{
    return m_minInterval;
}

float AutoFire::maxInterval() const
{
    return m_maxInterval;
}

float AutoFire::offsetX() const
{
    return m_offsetX;
}

float AutoFire::offsetY() const
{
    return m_offsetY;
}

bool AutoFire::started() const
{
    return m_started;
}

float AutoFire::timer() const
{
    return m_timer;
}

void AutoFire::restart(float timer)
{
    m_timer = timer;
    m_started = true;
}

void AutoFire::elapse(float dt)
{
    m_timer -= dt;
}

} // namespace engine::ecs
