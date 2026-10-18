#include "RenderSystem.hpp"

namespace rtype::client {

namespace {

constexpr engine::render::Color BACKGROUND{5, 5, 20, 255};

} // namespace

RenderSystem::RenderSystem(ClientWorld &world, engine::render::IRenderer &renderer) : m_world(world), m_renderer(renderer)
{
}

void RenderSystem::update(float)
{
    m_renderer.clear(BACKGROUND);
    for (std::size_t i : m_world.entities.with(POSITION | STAR)) {
        const Position &position = m_world.positions[i];
        const Star &star = m_world.stars[i];
        m_renderer.drawRect({position.x, position.y}, {star.size, star.size}, star.color);
    }
    for (std::size_t i : m_world.entities.with(POSITION | SPRITE)) {
        const Position &position = m_world.positions[i];
        const Sprite &sprite = m_world.sprites[i];
        m_renderer.drawTexture(sprite.texture, sprite.source, {position.x, position.y}, {sprite.scale, sprite.scale});
    }
    m_renderer.display();
}

} // namespace rtype::client
