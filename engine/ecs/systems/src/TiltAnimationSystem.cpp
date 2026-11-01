#include "TiltAnimationSystem.hpp"

#include "Sprite.hpp"
#include "TiltAnimation.hpp"

namespace engine::ecs {

TiltAnimationSystem::TiltAnimationSystem(Registry &registry) : m_registry(registry)
{
}

void TiltAnimationSystem::update(float dt)
{
    auto &animations = m_registry.getComponents<TiltAnimation>();
    auto &sprites = m_registry.getComponents<Sprite>();
    for (std::size_t i : m_registry.view<TiltAnimation, Sprite>()) {
        animations.at(i).advance(dt);
        sprites.at(i).setSource(animations.at(i).currentFrame());
    }
}

} // namespace engine::ecs
