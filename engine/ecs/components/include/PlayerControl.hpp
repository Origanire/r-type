#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class PlayerControl : public IComponent
{
public:
    explicit PlayerControl(float speed);

    float speed() const;
    void setDirection(bool up, bool down, bool left, bool right);
    float horizontal() const;
    float vertical() const;
    void setFiring(bool firing);
    bool firing() const;

private:
    float m_speed;
    float m_horizontal;
    float m_vertical;
    bool m_firing;
};

} // namespace engine::ecs
