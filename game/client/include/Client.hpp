#pragma once

#include <filesystem>
#include <unordered_map>

#include "FixedTimestep.hpp"
#include "InputManager.hpp"
#include "NetworkClient.hpp"
#include "Registry.hpp"
#include "Scheduler.hpp"
#include "SfmlRenderer.hpp"
#include "TextureManager.hpp"
#include "Window.hpp"

namespace rtype::client {

class Client
{
public:
    Client(const std::filesystem::path &assetsDir, engine::net::Endpoint server);

    void run();

private:
    void registerComponents();
    void registerSystems();
    void createStarfield();
    void applySnapshot(const protocol::WorldSnapshot &snapshot);

    engine::render::Window m_window;
    engine::render::TextureManager m_textures;
    engine::render::SfmlRenderer m_renderer;
    engine::input::InputManager m_input;
    engine::ecs::Registry m_registry;
    engine::ecs::Scheduler m_logic;
    engine::ecs::Scheduler m_render;
    engine::FixedTimestep m_timestep;
    NetworkClient m_network;
    std::unordered_map<std::uint16_t, engine::ecs::Entity> m_networkEntities;
};

} // namespace rtype::client
