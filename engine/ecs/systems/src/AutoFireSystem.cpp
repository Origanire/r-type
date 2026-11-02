#include "AutoFireSystem.hpp"

#include <string>
#include <utility>

#include "Owner.hpp"
#include "Transform.hpp"

namespace engine::ecs {

AutoFireSystem::AutoFireSystem(Registry &registry, IEntityFactory &factory, std::function<bool()> canSpawn)
    : m_registry(registry), m_factory(factory), m_canSpawn(std::move(canSpawn)), m_random(std::random_device{}())
{
}

void AutoFireSystem::update(float dt)
{
    for (std::size_t i : m_registry.view<AutoFire, Transform>()) {
        const Entity shooter = m_registry.entityAt(i);
        AutoFire &weapon = m_registry.get<AutoFire>(shooter);
        if (!weapon.started())
            weapon.restart(randomInterval(weapon));
        if (!m_canSpawn())
            continue;
        weapon.elapse(dt);
        if (weapon.timer() > 0.f)
            continue;
        weapon.restart(randomInterval(weapon));
        const Transform &origin = m_registry.get<Transform>(shooter);
        const float x = origin.x() + weapon.offsetX();
        const float y = origin.y() + weapon.offsetY();
        const std::string prefab = weapon.prefab();
        const Entity projectile = m_factory.spawn(prefab);
        if (m_registry.has<Transform>(projectile))
            m_registry.get<Transform>(projectile).setPosition(x, y);
        else
            m_registry.emplace<Transform>(projectile, x, y);
        m_registry.emplace<Owner>(projectile, shooter);
    }
}

float AutoFireSystem::randomInterval(const AutoFire &weapon)
{
    return std::uniform_real_distribution<float>(weapon.minInterval(), weapon.maxInterval())(m_random);
}

} // namespace engine::ecs
