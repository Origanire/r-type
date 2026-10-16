#pragma once

#include <cstdint>
#include <filesystem>
#include <unordered_map>

#include "ClientWorld.hpp"
#include "Entity.hpp"
#include "FixedTimestep.hpp"
#include "InputManager.hpp"
#include "NetworkClient.hpp"
#include "SfmlRenderer.hpp"
#include "SystemManager.hpp"
#include "TextureManager.hpp"
#include "Window.hpp"

namespace rtype::client {

class Client
{
public:
    Client(const std::filesystem::path &assetsDir, engine::net::Endpoint server);

    void run();

private:
    void registerSystems();
    void createStarfield();
    void applySnapshot(const protocol::WorldSnapshot &snapshot);
    engine::ecs::Entity createShip();

    engine::render::Window m_window;
    engine::render::TextureManager m_textures;
    engine::render::SfmlRenderer m_renderer;
    engine::input::InputManager m_input;
    ClientWorld m_world;
    engine::ecs::SystemManager m_logic;
    engine::ecs::SystemManager m_render;
    engine::FixedTimestep m_timestep;
    NetworkClient m_network;
    std::unordered_map<std::uint16_t, engine::ecs::Entity> m_networkEntities;
};

} // namespace rtype::client
