#include "Client.hpp"

#include <random>
#include <unordered_set>
#include <utility>

#include "Clock.hpp"
#include "GameConstants.hpp"
#include "Position.hpp"
#include "RenderSystem.hpp"
#include "ShipAnimationSystem.hpp"
#include "ShipTilt.hpp"
#include "Sprite.hpp"
#include "SpriteSheet.hpp"
#include "Star.hpp"
#include "StarfieldSystem.hpp"

namespace rtype::client {

using engine::ecs::Phase;

namespace {

constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int WINDOW_HEIGHT = 720;
constexpr float PLAYER_SCALE = PLAYER_WIDTH / static_cast<float>(PLAYER_SHIP_SHEET.frameWidth);
constexpr float MISSILE_SCALE = MISSILE_WIDTH / static_cast<float>(PLAYER_MISSILE_SHEET.frameWidth);

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

// The player slot picks the ship color, so each of the four players keeps the same ship.
int playerRow(const protocol::WorldSnapshot &snapshot, std::uint16_t entityId)
{
    for (std::size_t slot = 0; slot < snapshot.players.size(); ++slot) {
        if (snapshot.players[slot].state != protocol::SLOT_EMPTY && snapshot.players[slot].entityId == entityId)
            return static_cast<int>(slot);
    }
    return 0;
}

// Screen Y grows downwards: a smaller Y means the ship goes up.
float verticalDirection(float previousY, float nextY)
{
    if (nextY < previousY)
        return 1.f;
    if (nextY > previousY)
        return -1.f;
    return 0.f;
}

} // namespace

Client::Client(const std::filesystem::path &assetsDir, engine::net::Endpoint server)
    : m_window(WINDOW_WIDTH, WINDOW_HEIGHT, "R-Type", {WORLD_WIDTH, WORLD_HEIGHT}), m_renderer(m_window, m_textures),
      m_timestep(1.f / static_cast<float>(protocol::TICK_RATE)), m_network(std::move(server))
{
    m_textures.load(PLAYER_SHIP_SHEET.texture, (assetsDir / "sprites" / PLAYER_SHIP_SHEET.path).string());
    m_textures.load(PLAYER_MISSILE_SHEET.texture, (assetsDir / "sprites" / PLAYER_MISSILE_SHEET.path).string());
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
}

void Client::registerComponents()
{
    m_registry.registerComponent<Position>();
    m_registry.registerComponent<Sprite>();
    m_registry.registerComponent<Star>();
    m_registry.registerComponent<ShipTilt>();
}

void Client::registerSystems()
{
    m_logic.addSystem(Phase::Update, starfieldSystem);
    m_logic.addSystem(Phase::Update, shipAnimationSystem);
    m_render.addSystem(Phase::Render, RenderSystem(m_renderer));
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
    auto &positions = m_registry.getComponents<Position>();
    auto &tilts = m_registry.getComponents<ShipTilt>();
    for (const auto &remote : snapshot.entities) {
        const bool ship = remote.kind == protocol::KIND_PLAYER_SHIP;
        if (!ship && remote.kind != protocol::KIND_PLAYER_MISSILE)
            continue;
        visible.insert(remote.id);
        const Position next{static_cast<float>(remote.x), static_cast<float>(remote.y)};
        auto found = m_networkEntities.find(remote.id);
        if (found == m_networkEntities.end()) {
            const engine::ecs::Entity entity = ship ? createShip(playerRow(snapshot, remote.id), next) : createMissile(next);
            found = m_networkEntities.emplace(remote.id, entity).first;
        }
        if (ship)
            tilts[found->second]->direction = verticalDirection(positions[found->second]->y, next.y);
        positions[found->second] = next;
    }
    for (auto it = m_networkEntities.begin(); it != m_networkEntities.end();) {
        if (visible.contains(it->first)) {
            ++it;
            continue;
        }
        m_registry.killEntity(it->second);
        it = m_networkEntities.erase(it);
    }
}

engine::ecs::Entity Client::createShip(int row, Position position)
{
    engine::ecs::Entity ship = m_registry.spawnEntity();
    m_registry.addComponent(ship, position);
    m_registry.addComponent(
        ship, Sprite{PLAYER_SHIP_SHEET.texture, PLAYER_SHIP_SHEET.frame(PLAYER_SHIP_FLAT_COLUMN, row), PLAYER_SCALE});
    m_registry.addComponent(ship, ShipTilt{row});
    return ship;
}

engine::ecs::Entity Client::createMissile(Position position)
{
    engine::ecs::Entity missile = m_registry.spawnEntity();
    m_registry.addComponent(missile, position);
    m_registry.addComponent(missile, Sprite{PLAYER_MISSILE_SHEET.texture, PLAYER_MISSILE_SHEET.frame(0, 0), MISSILE_SCALE});
    return missile;
}

} // namespace rtype::client
