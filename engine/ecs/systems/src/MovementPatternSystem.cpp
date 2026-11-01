#include "MovementPatternSystem.hpp"

#include <cmath>

#include "Collider.hpp"
#include "MovementPattern.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"

namespace engine::ecs {

MovementPatternSystem::MovementPatternSystem(Registry &registry) : m_registry(registry)
{
}

void MovementPatternSystem::update(float dt)
{
    auto &patterns = m_registry.getComponents<MovementPattern>();
    const auto &transforms = m_registry.getComponents<Transform>();
    auto &velocities = m_registry.getComponents<Velocity>();
    const auto &colliders = m_registry.getComponents<Collider>();
    for (std::size_t i : m_registry.view<MovementPattern, Transform, Velocity, Collider>()) {
        MovementPattern &pattern = patterns.at(i);
        const Transform &transform = transforms.at(i);
        Velocity &velocity = velocities.at(i);
        if (!pattern.hasBaseY())
            pattern.setBaseY(transform.y());
        pattern.advance(dt);
        switch (pattern.kind()) {
            case MovementPattern::Kind::Straight:
                velocity.setY(0.f);
                break;
            case MovementPattern::Kind::Sine: {
                const float target =
                    pattern.baseY() + pattern.amplitude() * std::sin(pattern.frequency() * pattern.age());
                if (dt > 0.f)
                    velocity.setY((target - transform.y()) / dt);
                break;
            }
            case MovementPattern::Kind::Zigzag:
                if (transform.y() <= pattern.minY())
                    velocity.setY(pattern.verticalSpeed());
                else if (transform.y() + colliders.at(i).height() >= pattern.maxY())
                    velocity.setY(-pattern.verticalSpeed());
                break;
        }
    }
}

} // namespace engine::ecs
