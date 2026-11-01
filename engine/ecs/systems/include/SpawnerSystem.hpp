#pragma once

#include <functional>
#include <random>

#include "IEntityFactory.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"
#include "Spawner.hpp"

namespace engine::ecs {

class SpawnerSystem : public ISystem
{
public:
    SpawnerSystem(Registry &registry, IEntityFactory &factory, std::function<bool()> canSpawn);

    void update(float dt) override;

private:
    float randomInterval(const Spawner &spawner);
    const SpawnEntry &pickEntry(const Spawner &spawner);
    void spawn(const Spawner &spawner);

    Registry &m_registry;
    IEntityFactory &m_factory;
    std::function<bool()> m_canSpawn;
    std::mt19937 m_random;
};

} // namespace engine::ecs
