#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "CollisionEvent.hpp"
#include "DeathEvent.hpp"
#include "EventQueue.hpp"
#include "IAudio.hpp"
#include "IEntityFactory.hpp"
#include "IRenderer.hpp"
#include "Registry.hpp"

namespace engine::scene {

class SystemContext
{
public:
    SystemContext(ecs::Registry &registry, ecs::IEntityFactory &factory);

    ecs::Registry &registry() const;
    ecs::IEntityFactory &factory() const;
    EventQueue<ecs::CollisionEvent> &collisions();
    EventQueue<ecs::DeathEvent> &deaths();

    void setCanSpawn(std::function<bool()> canSpawn);
    const std::function<bool()> &canSpawn() const;
    void setRenderer(render::IRenderer &renderer);
    render::IRenderer &renderer() const;
    void setAudio(audio::IAudio &audio);
    audio::IAudio &audio() const;
    void setFlag(const std::string &name, bool value);
    bool flag(const std::string &name, bool fallback) const;

private:
    static bool alwaysAllowSpawn();

    ecs::Registry &m_registry;
    ecs::IEntityFactory &m_factory;
    EventQueue<ecs::CollisionEvent> m_collisions;
    EventQueue<ecs::DeathEvent> m_deaths;
    std::function<bool()> m_canSpawn;
    render::IRenderer *m_renderer;
    audio::IAudio *m_audio;
    std::unordered_map<std::string, bool> m_flags;
};

} // namespace engine::scene
