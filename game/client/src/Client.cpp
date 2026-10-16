#include "Client.hpp"

#include <memory>
#include <random>
#include <unordered_set>
#include <utility>

#include "Clock.hpp"
#include "GameConstants.hpp"
#include "Position.hpp"
#include "RenderSystem.hpp"
#include "Sprite.hpp"
#include "Star.hpp"
#include "StarfieldSystem.hpp"

namespace rtype::client {

namespace {

constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int WINDOW_HEIGHT = 720;
constexpr const char *PLAYER_TEXTURE = "player";
constexpr engine::render::IntRect PLAYER_FRAME{66, 0, 33, 17};
constexpr float PLAYER_SCALE = PLAYER_WIDTH / static_cast<float>(PLAYER_FRAME.width);

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
    : m_window(WINDOW_WIDTH, WINDOW_HEIGHT, "R-Type", {WORLD_WIDTH, WORLD_HEIGHT}), m_renderer(m_window, m_textures),
      m_timestep(1.f / static_cast<float>(protocol::TICK_RATE)), m_network(std::move(server))
{
    m_textures.load(PLAYER_TEXTURE, (assetsDir / "sprites" / "players" / "r-typesheet42.gif").string());
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
            m_logic.update(m_timestep.stepSeconds());
        m_render.update(0.f);
    }
    m_network.disconnect();
}

void Client::registerSystems()
{
    m_logic.add(std::make_unique<StarfieldSystem>(m_world));
    m_render.add(std::make_unique<RenderSystem>(m_world, m_renderer));
}

void Client::createStarfield()
{
    std::mt19937 random(std::random_device{}());
    std::uniform_real_distribution<float> randomX(0.f, WORLD_WIDTH);
    std::uniform_real_distribution<float> randomY(0.f, WORLD_HEIGHT);

    for (const StarLayer &layer : STAR_LAYERS) {
        for (int i = 0; i < layer.count; ++i) {
            engine::ecs::Entity star = m_world.entities.create();
            m_world.positions[star.index()] = Position{randomX(random), randomY(random)};
            m_world.stars[star.index()] = Star{layer.speed, layer.size, layer.color};
            m_world.entities.addComponents(star, POSITION | STAR);
        }
    }
}

void Client::applySnapshot(const protocol::WorldSnapshot &snapshot)
{
    std::unordered_set<std::uint16_t> visible;
    for (const auto &remote : snapshot.entities) {
        if (remote.kind != protocol::KIND_PLAYER_SHIP)
            continue;
        visible.insert(remote.id);
        auto found = m_networkEntities.find(remote.id);
        if (found == m_networkEntities.end())
            found = m_networkEntities.emplace(remote.id, createShip()).first;
        m_world.positions[found->second.index()] = Position{static_cast<float>(remote.x), static_cast<float>(remote.y)};
    }
    for (auto it = m_networkEntities.begin(); it != m_networkEntities.end();) {
        if (visible.contains(it->first)) {
            ++it;
            continue;
        }
        m_world.entities.destroy(it->second);
        it = m_networkEntities.erase(it);
    }
}

engine::ecs::Entity Client::createShip()
{
    engine::ecs::Entity ship = m_world.entities.create();
    m_world.positions[ship.index()] = Position{};
    m_world.sprites[ship.index()] = Sprite{PLAYER_TEXTURE, PLAYER_FRAME, PLAYER_SCALE};
    m_world.entities.addComponents(ship, POSITION | SPRITE);
    return ship;
}

} // namespace rtype::client
