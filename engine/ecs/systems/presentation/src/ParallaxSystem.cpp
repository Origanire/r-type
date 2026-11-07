#include "ParallaxSystem.hpp"

#include "Parallax.hpp"
#include "RectShape.hpp"
#include "Transform.hpp"

namespace engine::ecs {

ParallaxSystem::ParallaxSystem(Registry &registry) : m_registry(registry)
{
}

void ParallaxSystem::update(float dt)
{
    auto &transforms = m_registry.getComponents<Transform>();
    const auto &layers = m_registry.getComponents<Parallax>();
    const auto &shapes = m_registry.getComponents<RectShape>();
    for (std::size_t i : m_registry.view<Transform, Parallax, RectShape>()) {
        Transform &transform = transforms.at(i);
        const Parallax &layer = layers.at(i);
        const float width = shapes.at(i).width();
        transform.translate(-layer.speed() * dt, 0.f);
        if (transform.x() < -width)
            transform.setPosition(transform.x() + layer.wrapWidth() + width, transform.y());
    }
}

} // namespace engine::ecs
