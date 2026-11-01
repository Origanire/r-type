#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class MovementPatternSystem : public ISystem
{
public:
    explicit MovementPatternSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
