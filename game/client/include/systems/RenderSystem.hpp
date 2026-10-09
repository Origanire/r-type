#pragma once

#include "IRenderer.hpp"
#include "Position.hpp"
#include "SparseArray.hpp"
#include "Sprite.hpp"
#include "Star.hpp"
#include "SystemContext.hpp"

namespace rtype::client {

class RenderSystem
{
public:
    explicit RenderSystem(engine::render::IRenderer &renderer);

    void operator()(engine::ecs::SystemContext &context, engine::ecs::SparseArray<Position> &positions,
        engine::ecs::SparseArray<Star> &stars, engine::ecs::SparseArray<Sprite> &sprites) const;

private:
    engine::render::IRenderer &m_renderer;
};

} // namespace rtype::client
