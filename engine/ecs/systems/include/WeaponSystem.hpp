#pragma once

#include <functional>

#include "IEntityFactory.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class WeaponSystem : public ISystem
{
public:
    WeaponSystem(Registry &registry, IEntityFactory &factory, std::function<bool()> canSpawn);

    void update(float dt) override;

private:
    Registry &m_registry;
    IEntityFactory &m_factory;
    std::function<bool()> m_canSpawn;
};

} // namespace engine::ecs
