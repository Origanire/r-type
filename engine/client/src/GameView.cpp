#include "GameView.hpp"

#include <algorithm>
#include <charconv>
#include <system_error>

#include "ClientError.hpp"
#include "ComponentRegistration.hpp"
#include "DeathEvent.hpp"
#include "Destroyed.hpp"
#include "EngineComponentLoaders.hpp"
#include "EngineSystemBuilders.hpp"
#include "Lives.hpp"
#include "Button.hpp"
#include "HitFlash.hpp"
#include "UiFocus.hpp"
#include "Persistent.hpp"
#include "Protocol.hpp"
#include "Rotation.hpp"
#include "Player.hpp"
#include "PlayerIndex.hpp"
#include "Score.hpp"
#include "TextField.hpp"
#include "TiltAnimation.hpp"
#include "Transform.hpp"

namespace engine::client {

GameView::GameView(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
    const std::string &game)
    : m_game(scene::DataNode::fromFile(dataDirectory / (game + "_game.json"))),
      m_settings(loadLinked(dataDirectory, m_game, "settings")),
      m_window(static_cast<unsigned int>(m_game.child("window").getInt("width")),
          static_cast<unsigned int>(m_game.child("window").getInt("height")),
          m_game.child("window").getString("title"),
          {m_game.child("world").getFloat("width"), m_game.child("world").getFloat("height")}),
      m_renderer(m_window, m_textures, m_fonts), m_prefabs(m_registry, m_components),
      m_context(m_registry, m_prefabs), m_preload(), m_preloaded(0), m_loading(false),
      m_loadingFinished(false), m_loadingClock(), m_loadingMinimum(m_game.getFloat("loadingMinSeconds", 0.f))
{
    ecs::registerEngineComponents(m_registry);
    m_layers.load(m_game.child("layers"));
    if (m_game.has("worldValues"))
        m_worldTable.load(m_game.child("worldValues"));
    if (m_game.has("playerValues"))
        m_playerTable.load(m_game.child("playerValues"));
    if (m_game.has("rules"))
        m_ruleTable.load(m_game.child("rules"));
    if (m_game.has("eventSounds"))
        m_soundTable.load(m_game.child("eventSounds"));
    scene::EngineComponentLoaders::registerAll(m_components, m_layers);
    scene::EngineSystemBuilders::registerAll(m_systemFactory);
    m_prefabs.load(loadLinked(dataDirectory, m_game, "clientPrefabs"));
    m_settings.applyControls(m_input);
    m_renderer.setTextScale(m_settings.textScale());
    loadResources(assetsDirectory);
    loadKinds();
    m_context.setRenderer(m_renderer);
    m_context.setAudio(m_audio);
    m_context.setUiInput(m_uiInput);
    m_context.setUiStore(m_uiStore);
    m_context.setFlag("showPlayerLabels", m_settings.showPlayerLabels());
    m_systemFactory.build(m_context, m_game.child("renderSystems"), m_render);
    m_match = m_registry.spawn();
    m_registry.emplace<ecs::Match>(*m_match, 0.f);
    m_registry.emplace<ecs::Persistent>(*m_match);
    m_scenes = std::make_unique<SceneManager>(m_registry, m_prefabs, m_components, m_systemFactory, m_context,
        m_audio, Vector2(m_game.child("world").getFloat("width"), m_game.child("world").getFloat("height")),
        m_game.child("scenes"));
    if (m_game.has("loadingScene") && !m_preload.empty()) {
        m_loading = true;
        m_scenes->switchTo(m_game.getString("loadingScene"));
        return;
    }
    for (const std::string &name : m_preload)
        preload(name);
    m_scenes->switchTo(m_game.getString("startScene"));
}

bool GameView::isOpen() const
{
    return m_window.isOpen();
}

void GameView::closeWindow()
{
    m_window.close();
}

void GameView::processEvents()
{
    m_uiInput.beginFrame();
    m_window.processEvents(m_uiInput);
    m_input.update(m_window.hasFocus());
}

bool GameView::isPressed(input::Action action) const
{
    return m_input.isPressed(action);
}

bool GameView::escapePressed() const
{
    return m_uiInput.keyPressed(UiKey::Escape);
}

const PlayerSettings &GameView::settings() const
{
    return m_settings;
}

void GameView::switchScene(const std::string &name)
{
    clearRemoteWorld();
    m_scenes->switchTo(name);
}

const std::string &GameView::currentScene() const
{
    return m_scenes->current();
}

std::string GameView::escapeAction() const
{
    return m_scenes->escapeAction();
}

std::vector<ecs::UiActionEvent> GameView::takeUiActions()
{
    return m_context.uiActions().take();
}

ecs::UiStore &GameView::uiStore()
{
    return m_uiStore;
}

void GameView::setFieldText(const std::string &field, const std::string &text)
{
    auto &fields = m_registry.getComponents<ecs::TextField>();
    for (std::size_t i : m_registry.view<ecs::TextField>()) {
        if (fields.at(i).name() == field)
            fields.at(i).setContent(text);
    }
}

std::string GameView::message(const std::string &key) const
{
    if (!m_game.has("uiMessages") || !m_game.child("uiMessages").has(key))
        return key;
    return m_game.child("uiMessages").getString(key);
}

const scene::WorldValueTable &GameView::worldTable() const
{
    return m_worldTable;
}

void GameView::beginSync()
{
    m_seen.clear();
}

void GameView::syncEntity(std::uint16_t networkId, std::uint8_t kind, float x, float y, std::uint8_t visual)
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
    applyVisual(entity, visual);
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
    const float dt = std::min(m_clock.restart(), MAX_FRAME_SECONDS);
    if (m_loading)
        loadStep();
    m_scenes->update(dt);
    m_context.deaths().clear();
    m_render.update(dt);
}

void GameView::close()
{
    m_audio.stopMusic();
}

void GameView::loadResources(const std::filesystem::path &assetsDirectory)
{
    const scene::DataNode textures = m_game.child("textures");
    for (const std::string &name : textures.keys())
        m_textures.declare(name, (assetsDirectory / textures.getString(name)).string());
    if (m_game.has("fonts")) {
        const scene::DataNode fonts = m_game.child("fonts");
        for (const std::string &name : fonts.keys())
            m_fonts.declare(name, (assetsDirectory / fonts.getString(name)).string());
    }
    if (m_game.has("sounds")) {
        const scene::DataNode sounds = m_game.child("sounds");
        for (const std::string &name : sounds.keys())
            m_audio.declareSound(name, (assetsDirectory / sounds.getString(name)).string());
    }
    if (m_game.has("music")) {
        const scene::DataNode music = m_game.child("music");
        for (const std::string &name : music.keys())
            m_audio.declareMusic(name, (assetsDirectory / music.getString(name)).string());
    }
    if (m_game.has("preload")) {
        for (const scene::DataNode &name : m_game.child("preload").items())
            m_preload.push_back(name.asString());
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

void GameView::clearRemoteWorld()
{
    for (const auto &entry : m_networkEntities)
        m_registry.kill(entry.second);
    m_networkEntities.clear();
    for (std::size_t i : m_registry.view<ecs::Player>())
        m_registry.kill(m_registry.entityAt(i));
    m_registry.get<ecs::Match>(*m_match).setPhase(ecs::Match::Phase::Waiting);
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

std::uint8_t GameView::parseKind(const std::string &text)
{
    unsigned int value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value > MAX_KIND)
        throw ClientError("GameView: invalid entity kind " + text);
    return static_cast<std::uint8_t>(value);
}

float GameView::verticalDirection(float previousY, float nextY)
{
    if (nextY < previousY)
        return 1.f;
    if (nextY > previousY)
        return -1.f;
    return 0.f;
}

scene::DataNode GameView::loadLinked(
    const std::filesystem::path &dataDirectory, const scene::DataNode &game, const std::string &key)
{
    return scene::DataNode::fromFile(dataDirectory / game.child("files").getString(key));
}

const scene::WorldValueTable &GameView::playerTable() const
{
    return m_playerTable;
}

const scene::WorldValueTable &GameView::ruleTable() const
{
    return m_ruleTable;
}

void GameView::playEventSound(std::uint8_t index)
{
    if (index < m_soundTable.size())
        m_audio.playSound(m_soundTable.name(index));
}

void GameView::applyVisual(const ecs::Entity &entity, std::uint8_t visual)
{
    if (visual & network::Protocol::VISUAL_FLASH) {
        if (m_registry.has<ecs::HitFlash>(entity))
            m_registry.get<ecs::HitFlash>(entity).restart(ecs::HitFlash::DEFAULT_SECONDS);
        else
            m_registry.emplace<ecs::HitFlash>(entity, ecs::HitFlash::DEFAULT_SECONDS);
    }
    const auto step = static_cast<float>(visual & network::Protocol::VISUAL_ANGLE_MASK);
    if (step == 0.f && !m_registry.has<ecs::Rotation>(entity))
        return;
    const float degrees = step * 360.f / network::Protocol::VISUAL_ANGLE_STEPS;
    if (m_registry.has<ecs::Rotation>(entity))
        m_registry.get<ecs::Rotation>(entity).setDegrees(degrees);
    else
        m_registry.emplace<ecs::Rotation>(entity, degrees);
}

bool GameView::takeLoadingFinished()
{
    const bool finished = m_loadingFinished;
    m_loadingFinished = false;
    return finished;
}

bool GameView::debugPressed() const
{
    return m_uiInput.keyPressed(UiKey::Debug);
}

std::size_t GameView::entityCount() const
{
    return m_registry.view<ecs::Transform>().size();
}

void GameView::loadStep()
{
    Clock budget;
    while (m_preloaded < m_preload.size() && budget.elapsed() < LOADING_BUDGET_SECONDS)
        preload(m_preload[m_preloaded++]);
    m_uiStore.setValue(LOADING_KEY, std::to_string(static_cast<float>(m_preloaded) / static_cast<float>(m_preload.size())));
    if (m_preloaded < m_preload.size() || m_loadingClock.elapsed() < m_loadingMinimum)
        return;
    m_loading = false;
    m_loadingFinished = true;
    m_scenes->switchTo(m_game.getString("startScene"));
}

void GameView::preload(const std::string &name)
{
    if (m_textures.declared(name))
        m_textures.preload(name);
    else if (m_audio.hasSound(name))
        m_audio.preloadSound(name);
    else
        throw ClientError("GameView: cannot preload unknown resource " + name);
}

bool GameView::consolePressed() const
{
    return m_uiInput.keyPressed(UiKey::Console);
}

bool GameView::enterPressed() const
{
    return m_uiInput.keyPressed(UiKey::Enter);
}

void GameView::focusField(const std::string &field)
{
    auto &fields = m_registry.getComponents<ecs::TextField>();
    for (std::size_t i : m_registry.view<ecs::TextField>())
        fields.at(i).setActive(!field.empty() && fields.at(i).name() == field);
}

void GameView::setMusicVolume(float volume)
{
    m_audio.setMusicVolume(volume);
}

void GameView::setSoundVolume(float volume)
{
    m_audio.setSoundVolume(volume);
}

void GameView::focusAction(const std::string &action)
{
    const auto &buttons = m_registry.getComponents<ecs::Button>();
    for (std::size_t i : m_registry.view<ecs::Button>()) {
        if (buttons.at(i).action() == action)
            ecs::UiFocus::hover(m_registry, m_registry.entityAt(i));
    }
}

const ecs::UiStore &GameView::uiStore() const
{
    return m_uiStore;
}

} // namespace engine::client
