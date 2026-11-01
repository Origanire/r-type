#pragma once

#include <vector>

#include "IComponent.hpp"
#include "SpawnEntry.hpp"

namespace engine::ecs {

class Spawner : public IComponent
{
public:
    Spawner(std::vector<SpawnEntry> entries, float minInterval, float maxInterval, float x);

    const std::vector<SpawnEntry> &entries() const;
    float minInterval() const;
    float maxInterval() const;
    float x() const;
    bool started() const;
    float timer() const;
    void restart(float timer);
    void elapse(float dt);

private:
    std::vector<SpawnEntry> m_entries;
    float m_minInterval;
    float m_maxInterval;
    float m_x;
    float m_timer;
    bool m_started;
};

} // namespace engine::ecs
