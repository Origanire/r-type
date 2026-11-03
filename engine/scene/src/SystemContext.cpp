#include "SystemContext.hpp"

#include <utility>

#include "SceneError.hpp"

namespace engine::scene {

SystemContext::SystemContext(ecs::Registry &registry, ecs::IEntityFactory &factory)
    : m_registry(registry), m_factory(factory), m_canSpawn(&SystemContext::alwaysAllowSpawn), m_renderer(nullptr),
      m_audio(nullptr)
{
}

bool SystemContext::alwaysAllowSpawn()
{
    return true;
}

ecs::Registry &SystemContext::registry() const
{
    return m_registry;
}

ecs::IEntityFactory &SystemContext::factory() const
{
    return m_factory;
}

EventQueue<ecs::CollisionEvent> &SystemContext::collisions()
{
    return m_collisions;
}

EventQueue<ecs::DeathEvent> &SystemContext::deaths()
{
    return m_deaths;
}

void SystemContext::setCanSpawn(std::function<bool()> canSpawn)
{
    if (!canSpawn)
        throw SceneError("SystemContext: empty spawn rule");
    m_canSpawn = std::move(canSpawn);
}

const std::function<bool()> &SystemContext::canSpawn() const
{
    return m_canSpawn;
}

void SystemContext::setRenderer(render::IRenderer &renderer)
{
    m_renderer = &renderer;
}

render::IRenderer &SystemContext::renderer() const
{
    if (!m_renderer)
        throw SceneError("SystemContext: this program has no renderer");
    return *m_renderer;
}

void SystemContext::setAudio(audio::IAudio &audio)
{
    m_audio = &audio;
}

audio::IAudio &SystemContext::audio() const
{
    if (!m_audio)
        throw SceneError("SystemContext: this program has no audio");
    return *m_audio;
}

void SystemContext::setFlag(const std::string &name, bool value)
{
    m_flags.insert_or_assign(name, value);
}

bool SystemContext::flag(const std::string &name, bool fallback) const
{
    const auto found = m_flags.find(name);
    return found == m_flags.end() ? fallback : found->second;
}

} // namespace engine::scene
