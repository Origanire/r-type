#include "WeaponSystem.hpp"

#include <utility>

#include "Owner.hpp"
#include "PlayerControl.hpp"
#include "Transform.hpp"
#include "Weapon.hpp"

namespace engine::ecs {

WeaponSystem::WeaponSystem(Registry &registry, IEntityFactory &factory, std::function<bool()> canSpawn)
    : m_registry(registry), m_factory(factory), m_canSpawn(std::move(canSpawn))
{
}

void WeaponSystem::update(float dt)
{
    for (std::size_t i : m_registry.view<Weapon, PlayerControl, Transform>()) {
        const Entity shooter = m_registry.entityAt(i);
        Weapon &weapon = m_registry.get<Weapon>(shooter);
        weapon.elapse(dt);
        if (!m_registry.get<PlayerControl>(shooter).firing() || !weapon.ready() || !m_canSpawn())
            continue;
        weapon.trigger();
        const Transform &origin = m_registry.get<Transform>(shooter);
        const float x = origin.x() + weapon.offsetX();
        const float y = origin.y() + weapon.offsetY();
        const Entity projectile = m_factory.spawn(weapon.prefab());
        if (m_registry.has<Transform>(projectile))
            m_registry.get<Transform>(projectile).setPosition(x, y);
        else
            m_registry.emplace<Transform>(projectile, x, y);
        m_registry.emplace<Owner>(projectile, shooter);
    }
}

} // namespace engine::ecs
