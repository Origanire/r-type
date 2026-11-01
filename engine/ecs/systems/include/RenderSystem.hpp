#pragma once

#include "IRenderer.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"
#include "RenderTypes.hpp"

namespace engine::ecs {

class RenderSystem : public ISystem
{
public:
    RenderSystem(Registry &registry, render::IRenderer &renderer, render::Color background);

    void update(float dt) override;

private:
    Registry &m_registry;
    render::IRenderer &m_renderer;
    render::Color m_background;
};

} // namespace engine::ecs
