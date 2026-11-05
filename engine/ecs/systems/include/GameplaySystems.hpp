#pragma once

#include <functional>
#include <random>
#include <string>

#include "IAudio.hpp"
#include "IEntityFactory.hpp"
#include "IRenderer.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"
#include "UiInput.hpp"
#include "UiStore.hpp"
#include "Vector2.hpp"

namespace engine::ecs {

class AttachmentSystem : public ISystem
{
public:
    explicit AttachmentSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

class HitFlashSystem : public ISystem
{
public:
    explicit HitFlashSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

class SolidSystem : public ISystem
{
public:
    explicit SolidSystem(Registry &registry);

    void update(float dt) override;

private:
    void pushOut(std::size_t mover, std::size_t wall);

    Registry &m_registry;
};

class TilemapSystem : public ISystem
{
public:
    TilemapSystem(Registry &registry, IEntityFactory &factory, std::function<bool()> canSpawn, Vector2 area);

    void update(float dt) override;

private:
    void spawnColumn(const Entity &map, float x);

    Registry &m_registry;
    IEntityFactory &m_factory;
    std::function<bool()> m_canSpawn;
    Vector2 m_area;
};

class ParticleSystem : public ISystem
{
public:
    explicit ParticleSystem(Registry &registry);

    void update(float dt) override;

private:
    void emit(const Entity &source);

    Registry &m_registry;
    std::mt19937 m_random;
};

class LifetimeSystem : public ISystem
{
public:
    static constexpr float MAX_ALPHA = 255.f;

    explicit LifetimeSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

class MusicSystem : public ISystem
{
public:
    MusicSystem(Registry &registry, const UiStore &store, audio::IAudio &audio);

    void update(float dt) override;

private:
    Registry &m_registry;
    const UiStore &m_store;
    audio::IAudio &m_audio;
    std::string m_current;
};

class ShakeSystem : public ISystem
{
public:
    ShakeSystem(const UiStore &store, render::IRenderer &renderer, std::string key, float strength);

    void update(float dt) override;

private:
    const UiStore &m_store;
    render::IRenderer &m_renderer;
    std::string m_key;
    float m_strength;
    std::mt19937 m_random;
};

class GravitySystem : public ISystem
{
public:
    explicit GravitySystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

class BounceSystem : public ISystem
{
public:
    explicit BounceSystem(Registry &registry);

    void update(float dt) override;

private:
    void bounceOnBounds(std::size_t index);
    void bounceOnSolid(std::size_t index, std::size_t wall);

    Registry &m_registry;
};

class ChoiceSystem : public ISystem
{
public:
    ChoiceSystem(Registry &registry, const UiInput &input, UiStore &store);

    void update(float dt) override;

private:
    Registry &m_registry;
    const UiInput &m_input;
    UiStore &m_store;
};

} // namespace engine::ecs
