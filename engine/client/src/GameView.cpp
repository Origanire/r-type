#include "GameView.hpp"

#include <charconv>
#include <cstdint>
#include <system_error>

#include "ClientError.hpp"
#include "ComponentRegistration.hpp"
#include "DeathEvent.hpp"
#include "Destroyed.hpp"
#include "EngineComponentLoaders.hpp"
#include "EngineSystemBuilders.hpp"
#include "Lives.hpp"
#include "Persistent.hpp"
#include "Player.hpp"
#include "PlayerIndex.hpp"
#include "SceneBuilder.hpp"
#include "Score.hpp"
#include "TiltAnimation.hpp"
#include "Transform.hpp"

namespace engine::client {

namespace {

constexpr unsigned int MAX_KIND = UINT8_MAX;

std::uint8_t parseKind(const std::string &text)
{
    unsigned int value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value > MAX_KIND)
        throw ClientError("GameView: invalid entity kind " + text);
    return static_cast<std::uint8_t>(value);
}

float verticalDirection(float previousY, float nextY)
{
    if (nextY < previousY)
        return 1.f;
    if (nextY > previousY)
        return -1.f;
    return 0.f;
}

scene::DataNode loadGame(const std::filesystem::path &dataDirectory, const std::string &game)
{
    return scene::DataNode::fromFile(dataDirectory / (game + "_game.json"));
}

scene::DataNode loadLinked(
    const std::filesystem::path &dataDirectory, const scene::DataNode &game, const std::string &key)
{
    return scene::DataNode::fromFile(dataDirectory / game.child("files").getString(key));
}

} // namespace

GameView::GameView(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
    const std::string &game)
    : m_game(loadGame(dataDirectory, game)), m_settings(loadLinked(dataDirectory, m_game, "settings")),
      m_window(static_cast<unsigned int>(m_game.child("window").getInt("width")),
          static_cast<unsigned int>(m_game.child("window").getInt("height")),
          m_game.child("window").getString("title"),
          {m_game.child("world").getFloat("width"), m_game.child("world").getFloat("height")}),
      m_renderer(m_window, m_textures, m_fonts), m_prefabs(m_registry, m_components),
      m_context(m_registry, m_prefabs), m_timestep(1.f / static_cast<float>(m_game.getInt("tickRate")))
{
    ecs::registerEngineComponents(m_registry);
    m_layers.load(m_game.child("layers"));
    scene::EngineComponentLoaders::registerAll(m_components, m_layers);
    scene::EngineSystemBuilders::registerAll(m_systemFactory);
    m_prefabs.load(loadLinked(dataDirectory, m_game, "clientPrefabs"));
    m_settings.applyControls(m_input);
    m_renderer.setTextScale(m_settings.textScale());
    loadResources(assetsDirectory);
    loadKinds();
    m_context.setRenderer(m_renderer);
    m_context.setAudio(m_audio);
    m_context.setFlag("showPlayerLabels", m_settings.showPlayerLabels());
    m_systemFactory.build(m_context, m_game.child("clientSystems"), m_logic);
    m_systemFactory.build(m_context, m_game.child("renderSystems"), m_render);
    m_match = m_registry.spawn();
    m_registry.emplace<ecs::Match>(*m_match, 0.f);
    m_registry.emplace<ecs::Persistent>(*m_match);
    scene::SceneBuilder(m_registry, m_prefabs, m_components,
        {m_game.child("world").getFloat("width"), m_game.child("world").getFloat("height")})
        .build(m_game.child("clientScene"));
    if (m_game.has("startMusic"))
        m_audio.playMusic(m_game.getString("startMusic"));
}

bool GameView::isOpen() const
{
    return m_window.isOpen();
}

void GameView::processEvents()
{
    m_window.processEvents();
    m_input.update(m_window.hasFocus());
}

bool GameView::isPressed(input::Action action) const
{
    return m_input.isPressed(action);
}

void GameView::beginSync()
{
    m_seen.clear();
}

void GameView::syncEntity(std::uint16_t networkId, std::uint8_t kind, float x, float y)
{
    const auto prefab = m_kindPrefabs.find(kind);
    if (prefab == m_kindPrefabs.end())
        return;
    m_seen.insert(networkId);
    auto found = m_networkEntities.find(networkId);
    if (found == m_networkEntities.end())
        found = m_networkEntities.emplace(networkId, createRemote(prefab->second, x, y)).first;
    const ecs::Entity entity = found->second;
    ecs::Transform &transform = m_registry.get<ecs::Transform>(entity);
    if (m_registry.has<ecs::TiltAnimation>(entity))
        m_registry.get<ecs::TiltAnimation>(entity).setDirection(verticalDirection(transform.y(), y));
    transform.setPosition(x, y);
}

void GameView::endSync()
{
    for (auto it = m_networkEntities.begin(); it != m_networkEntities.end();) {
        if (m_seen.contains(it->first)) {
            ++it;
            continue;
        }
        m_registry.kill(it->second);
        it = m_networkEntities.erase(it);
    }
}

void GameView::destroyEntity(std::uint16_t networkId)
{
    const auto found = m_networkEntities.find(networkId);
    if (found == m_networkEntities.end())
        return;
    const ecs::Entity entity = found->second;
    m_networkEntities.erase(found);
    if (!m_registry.has<ecs::Destroyed>(entity))
        m_registry.emplace<ecs::Destroyed>(entity);
    m_context.deaths().push(ecs::DeathEvent(entity, std::nullopt));
}

void GameView::setPhase(ecs::Match::Phase phase)
{
    m_registry.get<ecs::Match>(*m_match).setPhase(phase);
}

void GameView::setPlayer(std::size_t slot, int lives, std::uint32_t score, std::uint16_t avatarId)
{
    std::optional<ecs::Entity> player = playerEntity(slot);
    if (!player) {
        player = m_registry.spawn();
        m_registry.emplace<ecs::Player>(*player, slot);
        m_registry.emplace<ecs::Lives>(*player, lives);
        m_registry.emplace<ecs::Score>(*player);
        m_registry.emplace<ecs::Persistent>(*player);
    }
    m_registry.get<ecs::Lives>(*player).setCount(lives);
    m_registry.get<ecs::Score>(*player).setPoints(score);
    const auto avatar = m_networkEntities.find(avatarId);
    if (avatarId != 0 && avatar != m_networkEntities.end() && !m_registry.has<ecs::PlayerIndex>(avatar->second))
        m_registry.emplace<ecs::PlayerIndex>(avatar->second, slot);
}

void GameView::removePlayer(std::size_t slot)
{
    if (const auto player = playerEntity(slot))
        m_registry.kill(*player);
}

void GameView::frame()
{
    m_timestep.addTime(m_clock.restart());
    while (m_timestep.step()) {
        m_logic.update(m_timestep.stepSeconds());
        m_context.deaths().clear();
    }
    m_render.update(0.f);
}

void GameView::close()
{
    m_audio.stopMusic();
}

void GameView::loadResources(const std::filesystem::path &assetsDirectory)
{
    const scene::DataNode textures = m_game.child("textures");
    for (const std::string &name : textures.keys())
        m_textures.load(name, (assetsDirectory / textures.getString(name)).string());
    if (m_game.has("fonts")) {
        const scene::DataNode fonts = m_game.child("fonts");
        for (const std::string &name : fonts.keys())
            m_fonts.load(name, (assetsDirectory / fonts.getString(name)).string());
    }
    if (m_game.has("sounds")) {
        const scene::DataNode sounds = m_game.child("sounds");
        for (const std::string &name : sounds.keys())
            m_audio.loadSound(name, (assetsDirectory / sounds.getString(name)).string());
    }
    if (m_game.has("music")) {
        const scene::DataNode music = m_game.child("music");
        for (const std::string &name : music.keys())
            m_audio.loadMusic(name, (assetsDirectory / music.getString(name)).string());
    }
    m_audio.setMusicVolume(m_settings.musicVolume());
    m_audio.setSoundVolume(m_settings.soundVolume());
}

void GameView::loadKinds()
{
    const scene::DataNode kinds = m_game.child("kinds");
    for (const std::string &kind : kinds.keys()) {
        const std::string prefab = kinds.getString(kind);
        if (!m_prefabs.has(prefab))
            throw ClientError("GameView: unknown prefab " + prefab + " for entity kind " + kind);
        m_kindPrefabs.insert_or_assign(parseKind(kind), prefab);
    }
}

ecs::Entity GameView::createRemote(const std::string &prefab, float x, float y)
{
    const ecs::Entity entity = m_prefabs.spawn(prefab);
    if (m_registry.has<ecs::Transform>(entity))
        m_registry.get<ecs::Transform>(entity).setPosition(x, y);
    else
        m_registry.emplace<ecs::Transform>(entity, x, y);
    return entity;
}

std::optional<ecs::Entity> GameView::playerEntity(std::size_t slot) const
{
    const auto &players = m_registry.getComponents<ecs::Player>();
    for (std::size_t i : m_registry.view<ecs::Player>()) {
        if (players.at(i).slot() == slot)
            return m_registry.entityAt(i);
    }
    return std::nullopt;
}

} // namespace engine::client
