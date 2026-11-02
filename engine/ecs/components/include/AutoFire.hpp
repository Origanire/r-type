#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class AutoFire : public IComponent
{
public:
    AutoFire(std::string prefab, float minInterval, float maxInterval, float offsetX, float offsetY);

    const std::string &prefab() const;
    float minInterval() const;
    float maxInterval() const;
    float offsetX() const;
    float offsetY() const;
    bool started() const;
    float timer() const;
    void restart(float timer);
    void elapse(float dt);

private:
    std::string m_prefab;
    float m_minInterval;
    float m_maxInterval;
    float m_offsetX;
    float m_offsetY;
    float m_timer;
    bool m_started;
};

} // namespace engine::ecs
