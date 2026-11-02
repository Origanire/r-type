#include "Spawner.hpp"

#include <utility>

namespace engine::ecs {

Spawner::Spawner(std::vector<SpawnEntry> entries, float minInterval, float maxInterval)
    : m_entries(std::move(entries)), m_minInterval(minInterval), m_maxInterval(maxInterval), m_timer(0.f),
      m_started(false)
{
}

const std::vector<SpawnEntry> &Spawner::entries() const
{
    return m_entries;
}

float Spawner::minInterval() const
{
    return m_minInterval;
}

float Spawner::maxInterval() const
{
    return m_maxInterval;
}

bool Spawner::started() const
{
    return m_started;
}

float Spawner::timer() const
{
    return m_timer;
}

void Spawner::restart(float timer)
{
    m_timer = timer;
    m_started = true;
}

void Spawner::elapse(float dt)
{
    m_timer -= dt;
}

} // namespace engine::ecs
