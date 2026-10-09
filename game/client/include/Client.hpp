#pragma once

#include <filesystem>

#include "FixedTimestep.hpp"
#include "InputManager.hpp"
#include "Registry.hpp"
#include "Scheduler.hpp"
#include "SfmlRenderer.hpp"
#include "TextureManager.hpp"
#include "Window.hpp"

namespace rtype::client {

class Client
{
public:
    explicit Client(const std::filesystem::path &assetsDir);

    void run();

private:
    void registerComponents();
    void registerSystems();
    void createStarfield();
    void createPlayer();

    engine::render::Window m_window;
    engine::render::TextureManager m_textures;
    engine::render::SfmlRenderer m_renderer;
    engine::input::InputManager m_input;
    engine::ecs::Registry m_registry;
    engine::ecs::Scheduler m_logic;
    engine::ecs::Scheduler m_render;
    engine::FixedTimestep m_timestep;
};

} // namespace rtype::client
