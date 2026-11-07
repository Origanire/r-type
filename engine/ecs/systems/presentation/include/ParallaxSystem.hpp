#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class ParallaxSystem : public ISystem
{
public:
    explicit ParallaxSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
