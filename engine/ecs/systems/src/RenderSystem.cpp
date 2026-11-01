#include "RenderSystem.hpp"

#include "RectShape.hpp"
#include "Sprite.hpp"
#include "Transform.hpp"

namespace engine::ecs {

RenderSystem::RenderSystem(Registry &registry, render::IRenderer &renderer, render::Color background)
    : m_registry(registry), m_renderer(renderer), m_background(background)
{
}

void RenderSystem::update(float)
{
    const auto &transforms = m_registry.getComponents<Transform>();
    const auto &shapes = m_registry.getComponents<RectShape>();
    const auto &sprites = m_registry.getComponents<Sprite>();
    m_renderer.clear(m_background);
    for (std::size_t i : m_registry.view<Transform, RectShape>()) {
        const Transform &transform = transforms.at(i);
        const RectShape &shape = shapes.at(i);
        m_renderer.drawRect({transform.x(), transform.y()}, {shape.width(), shape.height()}, shape.color());
    }
    for (std::size_t i : m_registry.view<Transform, Sprite>()) {
        const Transform &transform = transforms.at(i);
        const Sprite &sprite = sprites.at(i);
        m_renderer.drawTexture(
            sprite.texture(), sprite.source(), {transform.x(), transform.y()}, {sprite.scale(), sprite.scale()});
    }
    m_renderer.display();
}

} // namespace engine::ecs
