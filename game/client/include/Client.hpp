#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "ComponentFactory.hpp"
#include "DataNode.hpp"
#include "Entity.hpp"
#include "FixedTimestep.hpp"
#include "InputManager.hpp"
#include "LayerTable.hpp"
#include "NetworkClient.hpp"
#include "PrefabLibrary.hpp"
#include "Registry.hpp"
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
    void loadTextures(const std::filesystem::path &assetsDir);
    void loadKinds();
    void registerSystems();
    void createStarfield();
    void applySnapshot(const protocol::WorldSnapshot &snapshot);
    engine::ecs::Entity createRemote(const std::string &prefab, int row, float x, float y);

    engine::scene::DataNode m_config;
    engine::render::Window m_window;
    engine::render::TextureManager m_textures;
    engine::render::SfmlRenderer m_renderer;
    engine::input::InputManager m_input;
    engine::ecs::Registry m_registry;
    engine::scene::LayerTable m_layers;
    engine::scene::ComponentFactory m_factory;
    engine::scene::PrefabLibrary m_prefabs;
    engine::ecs::SystemManager m_logic;
    engine::ecs::SystemManager m_render;
    engine::FixedTimestep m_timestep;
    NetworkClient m_network;
    std::unordered_map<std::uint8_t, std::string> m_kindPrefabs;
    std::unordered_map<std::uint16_t, engine::ecs::Entity> m_networkEntities;
};

} // namespace rtype::client
