#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Health : public IComponent
{
public:
    explicit Health(int points);

    int points() const;
    void damage(int amount);
    bool isDead() const;

private:
    int m_points;
};

} // namespace engine::ecs
