#pragma once

#include "IRenderer.hpp"
#include "SystemContext.hpp"

namespace rtype::client {

class RenderSystem
{
public:
    explicit RenderSystem(engine::render::IRenderer &renderer);

    void operator()(engine::ecs::SystemContext &context) const;

private:
    engine::render::IRenderer &m_renderer;
};

} // namespace rtype::client
