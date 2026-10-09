#include "RenderSystem.hpp"

#include "Zipper.hpp"

namespace rtype::client {

RenderSystem::RenderSystem(engine::render::IRenderer &renderer) : m_renderer(renderer)
{
}

void RenderSystem::operator()(engine::ecs::SystemContext &, engine::ecs::SparseArray<Position> &positions,
    engine::ecs::SparseArray<Star> &stars, engine::ecs::SparseArray<Sprite> &sprites) const
{
    m_renderer.clear({5, 5, 20, 255});
    for (auto [position, star] : engine::ecs::Zipper(positions, stars))
        m_renderer.drawRect({position.x, position.y}, {star.size, star.size}, star.color);
    for (auto [position, sprite] : engine::ecs::Zipper(positions, sprites))
        m_renderer.drawTexture(sprite.texture, sprite.source, {position.x, position.y}, {sprite.scale, sprite.scale});
    m_renderer.display();
}

} // namespace rtype::client
