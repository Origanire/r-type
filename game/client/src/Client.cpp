#include "Client.hpp"

#include <memory>
#include <random>
#include <unordered_set>
#include <utility>

#include "Clock.hpp"
#include "GameConstants.hpp"
#include "RenderSystem.hpp"
#include "ShipAnimationSystem.hpp"
#include "SpriteSheet.hpp"
#include "StarfieldSystem.hpp"

namespace rtype::client {

namespace {

constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int WINDOW_HEIGHT = 720;
constexpr float PLAYER_SCALE = PLAYER_WIDTH / static_cast<float>(PLAYER_SHIP_SHEET.frameWidth);
constexpr float MISSILE_SCALE = MISSILE_WIDTH / static_cast<float>(PLAYER_MISSILE_SHEET.frameWidth);
constexpr float BYDO_SCALE = BYDO_WIDTH / static_cast<float>(BYDO_SHEET.frameWidth);

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

int playerRow(const protocol::WorldSnapshot &snapshot, std::uint16_t entityId)
{
    for (std::size_t slot = 0; slot < snapshot.players.size(); ++slot) {
        if (snapshot.players[slot].state != protocol::SLOT_EMPTY && snapshot.players[slot].entityId == entityId)
            return static_cast<int>(slot);
    }
    return 0;
}

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
    m_textures.load(BYDO_SHEET.texture, (assetsDir / "sprites" / BYDO_SHEET.path).string());
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
    m_logic.add(std::make_unique<ShipAnimationSystem>(m_world));
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
        const bool ship = remote.kind == protocol::KIND_PLAYER_SHIP;
        if (!ship && remote.kind != protocol::KIND_PLAYER_MISSILE && remote.kind != protocol::KIND_BYDO)
            continue;
        visible.insert(remote.id);
        const Position next{static_cast<float>(remote.x), static_cast<float>(remote.y)};
        auto found = m_networkEntities.find(remote.id);
        if (found == m_networkEntities.end()) {
            engine::ecs::Entity entity = ship ? createShip(playerRow(snapshot, remote.id), next)
                : remote.kind == protocol::KIND_BYDO ? createBydo(next)
                                                     : createMissile(next);
            found = m_networkEntities.emplace(remote.id, entity).first;
        }
        std::size_t i = found->second.index();
        if (ship)
            m_world.shipTilts[i].direction = verticalDirection(m_world.positions[i].y, next.y);
        m_world.positions[i] = next;
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

engine::ecs::Entity Client::createShip(int row, Position position)
{
    engine::ecs::Entity ship = m_world.entities.create();
    m_world.positions[ship.index()] = position;
    m_world.sprites[ship.index()] =
        Sprite{PLAYER_SHIP_SHEET.texture, PLAYER_SHIP_SHEET.frame(PLAYER_SHIP_FLAT_COLUMN, row), PLAYER_SCALE};
    m_world.shipTilts[ship.index()] = ShipTilt{row, 0.f, 0.f};
    m_world.entities.addComponents(ship, POSITION | SPRITE | SHIP_TILT);
    return ship;
}

engine::ecs::Entity Client::createMissile(Position position)
{
    engine::ecs::Entity missile = m_world.entities.create();
    m_world.positions[missile.index()] = position;
    m_world.sprites[missile.index()] =
        Sprite{PLAYER_MISSILE_SHEET.texture, PLAYER_MISSILE_SHEET.frame(0, 0), MISSILE_SCALE};
    m_world.entities.addComponents(missile, POSITION | SPRITE);
    return missile;
}

engine::ecs::Entity Client::createBydo(Position position)
{
    engine::ecs::Entity bydo = m_world.entities.create();
    m_world.positions[bydo.index()] = position;
    m_world.sprites[bydo.index()] = Sprite{BYDO_SHEET.texture, BYDO_SHEET.frame(0, 0), BYDO_SCALE};
    m_world.entities.addComponents(bydo, POSITION | SPRITE);
    return bydo;
}

} // namespace rtype::client
