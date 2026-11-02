#pragma once

#include <functional>
#include <random>

#include "AutoFire.hpp"
#include "IEntityFactory.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class AutoFireSystem : public ISystem
{
public:
    AutoFireSystem(Registry &registry, IEntityFactory &factory, std::function<bool()> canSpawn);

    void update(float dt) override;

private:
    float randomInterval(const AutoFire &weapon);

    Registry &m_registry;
    IEntityFactory &m_factory;
    std::function<bool()> m_canSpawn;
    std::mt19937 m_random;
};

} // namespace engine::ecs
