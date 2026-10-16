#pragma once

#include "ClientWorld.hpp"
#include "IRenderer.hpp"
#include "ISystem.hpp"

namespace rtype::client {

class RenderSystem : public engine::ecs::ISystem
{
public:
    RenderSystem(ClientWorld &world, engine::render::IRenderer &renderer);

    void update(float dt) override;

private:
    ClientWorld &m_world;
    engine::render::IRenderer &m_renderer;
};

} // namespace rtype::client
