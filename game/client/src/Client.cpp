#include "Client.hpp"

#include <random>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <utility>

#include "Clock.hpp"
#include "Position.hpp"
#include "RenderSystem.hpp"
#include "Sprite.hpp"
#include "Star.hpp"
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

Client::Client(const std::filesystem::path &assetsDir, engine::net::Endpoint server)
    : m_window(1280, 720, "R-Type", {WORLD_WIDTH, WORLD_HEIGHT}), m_renderer(m_window, m_textures),
      m_timestep(TICK_SECONDS), m_network(std::move(server))
{
    m_textures.load("player", (assetsDir / "sprites" / "players" / "r-typesheet42.gif").string());
    registerComponents();
    registerSystems();
    createStarfield();
}

void Client::run()
{
    engine::Clock clock;

    while (m_window.isOpen()) {
        m_window.processEvents();
        m_input.update(m_window.hasFocus());
        m_network.update(m_input.toBits());
        if (auto snapshot = m_network.takeSnapshot())
            applySnapshot(*snapshot);
        m_timestep.addTime(clock.restart());
        while (m_timestep.step())
            m_logic.run(m_registry, m_timestep.stepSeconds());
        m_render.run(m_registry, 0.f);
    }
    m_network.disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void Client::registerComponents()
{
    m_registry.registerComponent<Position>();
    m_registry.registerComponent<Sprite>();
    m_registry.registerComponent<Star>();
}

void Client::registerSystems()
{
    m_logic.addSystem<Position, Star>(Phase::Update, starfieldSystem);
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

void Client::applySnapshot(const protocol::WorldSnapshot &snapshot)
{
    std::unordered_set<std::uint16_t> visible;
    for (const auto &remote : snapshot.entities) {
        if (remote.kind != 1)
            continue;
        visible.insert(remote.id);
        auto found = m_networkEntities.find(remote.id);
        if (found == m_networkEntities.end()) {
            const auto ship = m_registry.spawnEntity();
            m_registry.addComponent(ship, Position{});
            m_registry.addComponent(ship, Sprite{"player", {66, 0, 33, 17}, 3.f});
            found = m_networkEntities.emplace(remote.id, ship).first;
        }
        auto &position = *m_registry.getComponents<Position>()[found->second];
        position = {static_cast<float>(remote.x), static_cast<float>(remote.y)};
    }
    for (auto it = m_networkEntities.begin(); it != m_networkEntities.end();) {
        if (!visible.contains(it->first)) {
            m_registry.killEntity(it->second);
            it = m_networkEntities.erase(it);
        } else
            ++it;
    }
}

} // namespace rtype::client
