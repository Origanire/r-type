#include "RenderSystem.hpp"

#include "Position.hpp"
#include "Registry.hpp"
#include "Sprite.hpp"
#include "Star.hpp"

namespace rtype::client {

namespace {

constexpr engine::render::Color BACKGROUND{5, 5, 20, 255};

} // namespace

RenderSystem::RenderSystem(engine::render::IRenderer &renderer) : m_renderer(renderer)
{
}

void RenderSystem::operator()(engine::ecs::SystemContext &context) const
{
    auto &positions = context.registry.getComponents<Position>();
    auto &stars = context.registry.getComponents<Star>();
    auto &sprites = context.registry.getComponents<Sprite>();
    m_renderer.clear(BACKGROUND);
    for (std::size_t i = 0; i < positions.size() && i < stars.size(); ++i) {
        if (positions[i] && stars[i])
            m_renderer.drawRect({positions[i]->x, positions[i]->y}, {stars[i]->size, stars[i]->size}, stars[i]->color);
    }
    for (std::size_t i = 0; i < positions.size() && i < sprites.size(); ++i) {
        if (positions[i] && sprites[i]) {
            const float scale = sprites[i]->scale;
            m_renderer.drawTexture(sprites[i]->texture, sprites[i]->source, {positions[i]->x, positions[i]->y}, {scale, scale});
        }
    }
    m_renderer.display();
}

} // namespace rtype::client
