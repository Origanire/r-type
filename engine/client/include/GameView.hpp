#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Action.hpp"
#include "Clock.hpp"
#include "ComponentFactory.hpp"
#include "DataNode.hpp"
#include "Entity.hpp"
#include "FixedTimestep.hpp"
#include "FontManager.hpp"
#include "InputManager.hpp"
#include "LayerTable.hpp"
#include "Match.hpp"
#include "PlayerSettings.hpp"
#include "PrefabLibrary.hpp"
#include "Registry.hpp"
#include "SfmlAudio.hpp"
#include "SfmlRenderer.hpp"
#include "SystemContext.hpp"
#include "SystemFactory.hpp"
#include "SystemManager.hpp"
#include "TextureManager.hpp"
#include "Window.hpp"

namespace engine::client {

class GameView
{
public:
    GameView(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
        const std::string &game);

    bool isOpen() const;
    void processEvents();
    bool isPressed(input::Action action) const;

    void beginSync();
    void syncEntity(std::uint16_t networkId, std::uint8_t kind, float x, float y);
    void endSync();
    void destroyEntity(std::uint16_t networkId);
    void setPhase(ecs::Match::Phase phase);
    void setPlayer(std::size_t slot, int lives, std::uint32_t score, std::uint16_t avatarId);
    void removePlayer(std::size_t slot);

    void frame();
    void close();

private:
    void loadResources(const std::filesystem::path &assetsDirectory);
    void loadKinds();
    ecs::Entity createRemote(const std::string &prefab, float x, float y);
    std::optional<ecs::Entity> playerEntity(std::size_t slot) const;

    scene::DataNode m_game;
    PlayerSettings m_settings;
    render::Window m_window;
    render::TextureManager m_textures;
    render::FontManager m_fonts;
    render::SfmlRenderer m_renderer;
    audio::SfmlAudio m_audio;
    input::InputManager m_input;
    ecs::Registry m_registry;
    scene::LayerTable m_layers;
    scene::ComponentFactory m_components;
    scene::PrefabLibrary m_prefabs;
    scene::SystemFactory m_systemFactory;
    scene::SystemContext m_context;
    ecs::SystemManager m_logic;
    ecs::SystemManager m_render;
    FixedTimestep m_timestep;
    Clock m_clock;
    std::optional<ecs::Entity> m_match;
    std::unordered_map<std::uint8_t, std::string> m_kindPrefabs;
    std::unordered_map<std::uint16_t, ecs::Entity> m_networkEntities;
    std::unordered_set<std::uint16_t> m_seen;
};

} // namespace engine::client
