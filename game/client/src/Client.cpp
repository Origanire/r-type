#include "Client.hpp"

#include <random>

#include "Clock.hpp"
#include "MovementSystem.hpp"
#include "PlayerControlSystem.hpp"
#include "RenderSystem.hpp"
#include "ScreenClampSystem.hpp"
#include "StarfieldSystem.hpp"
#include "WorldSize.hpp"

namespace rtype::client {

using engine::ecs::Phase;

namespace {

constexpr float TICK_SECONDS = 1.f / 60.f;

struct StarLayer
{
    int count;
    float speed;
    float size;
    engine::render::Color color;
};

constexpr StarLayer STAR_LAYERS[] = {
    {120, 40.f, 2.f, {90, 90, 130, 255}},
    {60, 110.f, 3.f, {170, 170, 210, 255}},
    {25, 240.f, 4.f, {255, 255, 255, 255}},
};

} // namespace

Client::Client(const std::filesystem::path &assetsDir)
    : m_window(1280, 720, "R-Type", {WORLD_WIDTH, WORLD_HEIGHT}), m_renderer(m_window, m_textures),
      m_timestep(TICK_SECONDS)
{
    m_textures.load("player", (assetsDir / "sprites" / "players" / "r-typesheet42.gif").string());
    registerComponents();
    registerSystems();
    createStarfield();
    createPlayer();
}

void Client::run()
{
    engine::Clock clock;

    while (m_window.isOpen()) {
        m_window.processEvents();
        m_input.update(m_window.hasFocus());
        m_timestep.addTime(clock.restart());
        while (m_timestep.step())
            m_logic.run(m_registry, m_timestep.stepSeconds());
        m_render.run(m_registry, 0.f);
    }
}

void Client::registerComponents()
{
    m_registry.registerComponent<Position>();
    m_registry.registerComponent<Velocity>();
    m_registry.registerComponent<Sprite>();
    m_registry.registerComponent<Star>();
    m_registry.registerComponent<PlayerControl>();
}

void Client::registerSystems()
{
    m_logic.addSystem<PlayerControl, Velocity>(Phase::Input, PlayerControlSystem(m_input));
    m_logic.addSystem<Position, Star>(Phase::Update, starfieldSystem);
    m_logic.addSystem<Position, Velocity>(Phase::Physics, movementSystem);
    m_logic.addSystem<Position, Sprite, PlayerControl>(Phase::Physics, screenClampSystem);
    m_render.addSystem<Position, Star, Sprite>(Phase::Render, RenderSystem(m_renderer));
}

void Client::createStarfield()
{
    std::mt19937 random(std::random_device{}());
    std::uniform_real_distribution<float> randomX(0.f, WORLD_WIDTH);
    std::uniform_real_distribution<float> randomY(0.f, WORLD_HEIGHT);

    for (const StarLayer &layer : STAR_LAYERS) {
        for (int i = 0; i < layer.count; ++i) {
            engine::ecs::Entity star = m_registry.spawnEntity();
            m_registry.addComponent(star, Position{randomX(random), randomY(random)});
            m_registry.addComponent(star, Star{layer.speed, layer.size, layer.color});
        }
    }
}

void Client::createPlayer()
{
    engine::ecs::Entity player = m_registry.spawnEntity();
    m_registry.addComponent(player, Position{200.f, WORLD_HEIGHT / 2.f});
    m_registry.addComponent(player, Velocity{});
    m_registry.addComponent(player, Sprite{"player", {66, 0, 33, 17}, 3.f});
    m_registry.addComponent(player, PlayerControl{});
}

} // namespace rtype::client
