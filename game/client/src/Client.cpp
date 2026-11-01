#include "Client.hpp"

#include <charconv>
#include <cstddef>
#include <memory>
#include <random>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Clock.hpp"
#include "Color.hpp"
#include "ComponentRegistration.hpp"
#include "EngineComponentLoaders.hpp"
#include "GameError.hpp"
#include "Parallax.hpp"
#include "ParallaxSystem.hpp"
#include "RectShape.hpp"
#include "RenderSystem.hpp"
#include "Sprite.hpp"
#include "TiltAnimation.hpp"
#include "TiltAnimationSystem.hpp"
#include "Transform.hpp"

namespace rtype::client {

namespace {

constexpr std::size_t COLOR_VALUES = 4;

engine::Color readColor(const engine::scene::DataNode &values)
{
    const std::vector<engine::scene::DataNode> channels = values.items();
    if (channels.size() != COLOR_VALUES)
        throw GameError("Client: " + values.path() + " must be [r, g, b, a]");
    return {static_cast<std::uint8_t>(channels[0].asInt()), static_cast<std::uint8_t>(channels[1].asInt()),
        static_cast<std::uint8_t>(channels[2].asInt()), static_cast<std::uint8_t>(channels[3].asInt())};
}

std::uint8_t parseKind(const std::string &text)
{
    unsigned value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value > UINT8_MAX)
        throw GameError("Client: invalid entity kind " + text);
    return static_cast<std::uint8_t>(value);
}

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

engine::scene::DataNode loadConfig(const std::filesystem::path &dataDir)
{
    return engine::scene::DataNode::fromFile(dataDir / "rtype_game.json");
}

} // namespace

Client::Client(const std::filesystem::path &assetsDir, const std::filesystem::path &dataDir, engine::net::Endpoint server)
    : m_config(loadConfig(dataDir)),
      m_window(static_cast<unsigned int>(m_config.child("window").getInt("width")),
          static_cast<unsigned int>(m_config.child("window").getInt("height")),
          m_config.child("window").getString("title"),
          {m_config.child("world").getFloat("width"), m_config.child("world").getFloat("height")}),
      m_renderer(m_window, m_textures), m_prefabs(m_registry, m_factory),
      m_timestep(1.f / static_cast<float>(protocol::TICK_RATE)), m_network(std::move(server))
{
    engine::ecs::registerEngineComponents(m_registry);
    m_layers.load(m_config.child("layers"));
    engine::scene::addEngineComponentLoaders(m_factory, m_layers);
    m_prefabs.load(engine::scene::DataNode::fromFile(dataDir / "rtype_client_prefabs.json"));
    loadTextures(assetsDir);
    loadKinds();
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

void Client::loadTextures(const std::filesystem::path &assetsDir)
{
    const engine::scene::DataNode textures = m_config.child("textures");
    for (const std::string &name : textures.keys())
        m_textures.load(name, (assetsDir / textures.getString(name)).string());
}

void Client::loadKinds()
{
    const engine::scene::DataNode kinds = m_config.child("kinds");
    for (const std::string &kind : kinds.keys()) {
        const std::string prefab = kinds.getString(kind);
        if (!m_prefabs.has(prefab))
            throw GameError("Client: unknown prefab " + prefab + " for entity kind " + kind);
        m_kindPrefabs.insert_or_assign(parseKind(kind), prefab);
    }
}

void Client::registerSystems()
{
    m_logic.add(std::make_unique<engine::ecs::ParallaxSystem>(m_registry));
    m_logic.add(std::make_unique<engine::ecs::TiltAnimationSystem>(m_registry));
    m_render.add(std::make_unique<engine::ecs::RenderSystem>(
        m_registry, m_renderer, readColor(m_config.child("background"))));
}

void Client::createStarfield()
{
    const float worldWidth = m_config.child("world").getFloat("width");
    const float worldHeight = m_config.child("world").getFloat("height");
    std::mt19937 random(std::random_device{}());
    std::uniform_real_distribution<float> randomX(0.f, worldWidth);
    std::uniform_real_distribution<float> randomY(0.f, worldHeight);

    for (const engine::scene::DataNode &layer : m_config.child("starfield").items()) {
        const int count = layer.getInt("count");
        const float size = layer.getFloat("size");
        const engine::Color color = readColor(layer.child("color"));
        for (int i = 0; i < count; ++i) {
            const engine::ecs::Entity star = m_registry.spawn();
            m_registry.emplace<engine::ecs::Transform>(star, randomX(random), randomY(random));
            m_registry.emplace<engine::ecs::RectShape>(star, size, size, color);
            m_registry.emplace<engine::ecs::Parallax>(star, layer.getFloat("speed"), worldWidth);
        }
    }
}

void Client::applySnapshot(const protocol::WorldSnapshot &snapshot)
{
    std::unordered_set<std::uint16_t> visible;
    for (const auto &remote : snapshot.entities) {
        const auto prefab = m_kindPrefabs.find(remote.kind);
        if (prefab == m_kindPrefabs.end())
            continue;
        visible.insert(remote.id);
        const float x = static_cast<float>(remote.x);
        const float y = static_cast<float>(remote.y);
        auto found = m_networkEntities.find(remote.id);
        if (found == m_networkEntities.end()) {
            const int row = remote.kind == protocol::KIND_PLAYER_SHIP ? playerRow(snapshot, remote.id) : 0;
            found = m_networkEntities.emplace(remote.id, createRemote(prefab->second, row, x, y)).first;
        }
        const engine::ecs::Entity entity = found->second;
        auto &transform = m_registry.get<engine::ecs::Transform>(entity);
        if (m_registry.has<engine::ecs::TiltAnimation>(entity))
            m_registry.get<engine::ecs::TiltAnimation>(entity).setDirection(verticalDirection(transform.y(), y));
        transform.setPosition(x, y);
    }
    for (auto it = m_networkEntities.begin(); it != m_networkEntities.end();) {
        if (visible.contains(it->first)) {
            ++it;
            continue;
        }
        m_registry.kill(it->second);
        it = m_networkEntities.erase(it);
    }
}

engine::ecs::Entity Client::createRemote(const std::string &prefab, int row, float x, float y)
{
    const engine::ecs::Entity entity = m_prefabs.spawn(prefab);
    if (m_registry.has<engine::ecs::Transform>(entity))
        m_registry.get<engine::ecs::Transform>(entity).setPosition(x, y);
    else
        m_registry.emplace<engine::ecs::Transform>(entity, x, y);
    if (m_registry.has<engine::ecs::TiltAnimation>(entity) && m_registry.has<engine::ecs::Sprite>(entity)) {
        auto &animation = m_registry.get<engine::ecs::TiltAnimation>(entity);
        animation.setRow(row);
        m_registry.get<engine::ecs::Sprite>(entity).setSource(animation.currentFrame());
    }
    return entity;
}

} // namespace rtype::client
