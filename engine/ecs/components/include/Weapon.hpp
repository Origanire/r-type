#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class Weapon : public IComponent
{
public:
    Weapon(std::string prefab, float cooldown, float offsetX, float offsetY);

    const std::string &prefab() const;
    float offsetX() const;
    float offsetY() const;
    void elapse(float dt);
    bool ready() const;
    void trigger();

private:
    std::string m_prefab;
    float m_cooldown;
    float m_offsetX;
    float m_offsetY;
    float m_remaining;
};

} // namespace engine::ecs
