#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "ClientSettings.hpp"
#include "ComponentFactory.hpp"
#include "DataNode.hpp"
#include "Entity.hpp"
#include "FixedTimestep.hpp"
#include "FontManager.hpp"
#include "Hud.hpp"
#include "InputManager.hpp"
#include "LayerTable.hpp"
#include "NetworkClient.hpp"
#include "PrefabLibrary.hpp"
#include "Registry.hpp"
#include "SfmlAudio.hpp"
#include "SfmlRenderer.hpp"
#include "SystemManager.hpp"
#include "TextureManager.hpp"
#include "Window.hpp"

namespace rtype::client {

class Client
{
public:
    Client(const std::filesystem::path &assetsDir, const std::filesystem::path &dataDir, engine::net::Endpoint server);

    void run();

private:
    void loadResources(const std::filesystem::path &assetsDir);
    void loadKinds();
    void registerSystems();
    void createStarfield();
    void handleEvent(const protocol::Packet &event);
    void explodeAt(const engine::ecs::Entity &entity);
    void applySnapshot(const protocol::WorldSnapshot &snapshot);
    engine::ecs::Entity createRemote(const std::string &prefab, int row, float x, float y);
    void attachLabel(const engine::ecs::Entity &ship, int row);

    engine::scene::DataNode m_config;
    ClientSettings m_settings;
    engine::render::Window m_window;
    engine::render::TextureManager m_textures;
    engine::render::FontManager m_fonts;
    engine::render::SfmlRenderer m_renderer;
    engine::audio::SfmlAudio m_audio;
    engine::input::InputManager m_input;
    engine::ecs::Registry m_registry;
    engine::scene::LayerTable m_layers;
    engine::scene::ComponentFactory m_factory;
    engine::scene::PrefabLibrary m_prefabs;
    engine::ecs::SystemManager m_logic;
    engine::ecs::SystemManager m_render;
    engine::FixedTimestep m_timestep;
    NetworkClient m_network;
    std::unique_ptr<Hud> m_hud;
    std::unordered_map<std::uint8_t, std::string> m_kindPrefabs;
    std::unordered_map<std::uint16_t, engine::ecs::Entity> m_networkEntities;
};

} // namespace rtype::client
