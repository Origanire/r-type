#include "RenderSystem.hpp"

#include "Foreground.hpp"
#include "Hidden.hpp"
#include "HitFlash.hpp"
#include "Rotation.hpp"
#include "RectShape.hpp"
#include "Sprite.hpp"
#include "Text.hpp"
#include "Transform.hpp"

namespace engine::ecs {

RenderSystem::RenderSystem(Registry &registry, render::IRenderer &renderer, Color background)
    : m_registry(registry), m_renderer(renderer), m_background(background)
{
}

void RenderSystem::update(float)
{
    m_renderer.clear(m_background);
    const Vector2 shake = m_renderer.offset();
    drawLayer(false);
    m_renderer.setOffset({0.f, 0.f});
    drawLayer(true);
    drawTexts();
    m_renderer.setOffset(shake);
    m_renderer.display();
}

void RenderSystem::drawLayer(bool foreground)
{
    const auto &transforms = m_registry.getComponents<Transform>();
    const auto &shapes = m_registry.getComponents<RectShape>();
    const auto &sprites = m_registry.getComponents<Sprite>();
    for (std::size_t i : m_registry.view<Transform, RectShape>()) {
        if (isHidden(i) || isForeground(i) != foreground)
            continue;
        const Transform &transform = transforms.at(i);
        const RectShape &shape = shapes.at(i);
        m_renderer.drawRect({transform.x(), transform.y()}, {shape.width(), shape.height()}, shape.color());
    }
    for (std::size_t i : m_registry.view<Transform, Sprite>()) {
        if (isHidden(i) || isForeground(i) != foreground)
            continue;
        const Transform &transform = transforms.at(i);
        const Sprite &sprite = sprites.at(i);
        m_renderer.drawTexture(sprite.texture(), sprite.source(), {transform.x(), transform.y()},
            {sprite.scale(), sprite.scale()}, rotationOf(i), tintOf(i));
    }
}

void RenderSystem::drawTexts()
{
    const auto &transforms = m_registry.getComponents<Transform>();
    const auto &texts = m_registry.getComponents<Text>();
    for (std::size_t i : m_registry.view<Transform, Text>()) {
        const Text &text = texts.at(i);
        if (isHidden(i) || text.content().empty())
            continue;
        const Transform &transform = transforms.at(i);
        m_renderer.drawText(text.font(), text.content(), text.size(),
            {transform.x() + text.offsetX(), transform.y() + text.offsetY()}, text.color());
    }
}

bool RenderSystem::isHidden(std::size_t index) const
{
    return m_registry.has<Hidden>(m_registry.entityAt(index));
}

bool RenderSystem::isForeground(std::size_t index) const
{
    return m_registry.has<Foreground>(m_registry.entityAt(index));
}

float RenderSystem::rotationOf(std::size_t index) const
{
    const Entity entity = m_registry.entityAt(index);
    return m_registry.has<Rotation>(entity) ? m_registry.get<Rotation>(entity).degrees() : 0.f;
}

Color RenderSystem::tintOf(std::size_t index) const
{
    return m_registry.has<HitFlash>(m_registry.entityAt(index)) ? Color(FLASH_RED, FLASH_OTHER, FLASH_OTHER)
                                                                  : Color();
}

} // namespace engine::ecs
