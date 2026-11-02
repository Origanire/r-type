#include "SpawnerSystem.hpp"

#include <utility>

#include "EcsError.hpp"
#include "Transform.hpp"

namespace engine::ecs {

SpawnerSystem::SpawnerSystem(Registry &registry, IEntityFactory &factory, std::function<bool()> canSpawn)
    : m_registry(registry), m_factory(factory), m_canSpawn(std::move(canSpawn)), m_random(std::random_device{}())
{
}

void SpawnerSystem::update(float dt)
{
    auto &spawners = m_registry.getComponents<Spawner>();
    for (std::size_t i : m_registry.view<Spawner>()) {
        Spawner &spawner = spawners.at(i);
        if (!spawner.started())
            spawner.restart(randomInterval(spawner));
        if (!m_canSpawn())
            continue;
        spawner.elapse(dt);
        if (spawner.timer() > 0.f)
            continue;
        spawner.restart(randomInterval(spawner));
        spawn(spawner);
    }
}

float SpawnerSystem::randomInterval(const Spawner &spawner)
{
    return std::uniform_real_distribution<float>(spawner.minInterval(), spawner.maxInterval())(m_random);
}

const SpawnEntry &SpawnerSystem::pickEntry(const Spawner &spawner)
{
    const auto &entries = spawner.entries();
    if (entries.empty())
        throw EcsError("SpawnerSystem: a spawner has no entry");
    float total = 0.f;
    for (const SpawnEntry &entry : entries)
        total += entry.weight();
    float roll = std::uniform_real_distribution<float>(0.f, total)(m_random);
    for (const SpawnEntry &entry : entries) {
        roll -= entry.weight();
        if (roll <= 0.f)
            return entry;
    }
    return entries.back();
}

void SpawnerSystem::spawn(const Spawner &spawner)
{
    const SpawnEntry &entry = pickEntry(spawner);
    const float x = std::uniform_real_distribution<float>(entry.minX(), entry.maxX())(m_random);
    const float y = std::uniform_real_distribution<float>(entry.minY(), entry.maxY())(m_random);
    const Entity entity = m_factory.spawn(entry.prefab());
    if (m_registry.has<Transform>(entity))
        m_registry.get<Transform>(entity).setPosition(x, y);
    else
        m_registry.emplace<Transform>(entity, x, y);
}

} // namespace engine::ecs
