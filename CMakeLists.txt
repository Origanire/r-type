cmake_minimum_required(VERSION 3.21)

project(RType
    VERSION 0.1.0
    DESCRIPTION "Networked multiplayer shoot'em up built on a custom game engine"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Each target below is commented out until its sources exist: uncomment a
# block when the first files of that module are written.
#
# Rules [p.12][p.15]:
#   - engine targets never link anything from game/
#   - r-type_server never links engine_render, engine_audio, engine_input or engine_ui

# ---------------------------------------------------------------------------
# Dependencies (vcpkg.json). Check the exact target names printed by vcpkg.
# ---------------------------------------------------------------------------

find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
# Add Audio to the components once engine_audio is written
find_package(SFML 3 COMPONENTS Graphics Window System CONFIG REQUIRED)

# ---------------------------------------------------------------------------
# Engine: knows nothing about R-Type. One module = one library [p.16].
# ---------------------------------------------------------------------------

# --- engine_core: errors, clock, logger, event bus, thread-safe queue, config, paths
# Planned: Logger.cpp, Random.cpp, Config.cpp, Paths.cpp
add_library(engine_core
    engine/core/src/EngineError.cpp
    engine/core/src/Clock.cpp
    engine/core/src/FixedTimestep.cpp
)
target_include_directories(engine_core PUBLIC engine/core/include)
# Uncomment once Config.cpp exists
# target_link_libraries(engine_core PRIVATE nlohmann_json::nlohmann_json)

# --- engine_ecs: entities, components, systems
add_library(engine_ecs
    engine/ecs/src/Entity.cpp
    engine/ecs/src/Registry.cpp
    engine/ecs/src/CommandBuffer.cpp
    engine/ecs/src/Scheduler.cpp
)
target_include_directories(engine_ecs PUBLIC engine/ecs/include)
target_link_libraries(engine_ecs PUBLIC engine_core)

# --- engine_physics: transform, velocity, hitboxes, movement, collisions
# add_library(engine_physics
#     engine/physics/src/Aabb.cpp
#     engine/physics/src/MovementSystem.cpp
#     engine/physics/src/CollisionSystem.cpp
# )
# target_include_directories(engine_physics PUBLIC engine/physics/include)
# target_link_libraries(engine_physics PUBLIC engine_core engine_ecs)

# --- engine_net: serialization, UDP client/server (Asio hidden inside) [p.9][p.11]
# add_library(engine_net
#     engine/net/src/Writer.cpp
#     engine/net/src/Reader.cpp
#     engine/net/src/PacketHeader.cpp
#     engine/net/src/Endpoint.cpp
#     engine/net/src/UdpServer.cpp
#     engine/net/src/UdpClient.cpp
#     engine/net/src/SessionManager.cpp
#     engine/net/src/RateLimiter.cpp
#     engine/net/src/ReliableChannel.cpp
#     engine/net/src/NetStats.cpp
# )
# target_include_directories(engine_net PUBLIC engine/net/include)
# target_link_libraries(engine_net PUBLIC engine_core PRIVATE asio::asio)
# target_compile_definitions(engine_net PRIVATE ASIO_STANDALONE)

# --- engine_render: window, SFML renderer, textures, fonts, palettes (client only)
# Planned: FontManager.cpp, Palette.cpp
add_library(engine_render
    engine/render/src/SfmlRenderer.cpp
    engine/render/src/Window.cpp
    engine/render/src/TextureManager.cpp
)
target_include_directories(engine_render PUBLIC engine/render/include)
target_link_libraries(engine_render PUBLIC engine_core SFML::Graphics)

# --- engine_audio: sounds and music (client only)
# add_library(engine_audio
#     engine/audio/src/SfmlAudio.cpp
#     engine/audio/src/SoundManager.cpp
#     engine/audio/src/MusicPlayer.cpp
# )
# target_include_directories(engine_audio PUBLIC engine/audio/include)
# target_link_libraries(engine_audio PUBLIC engine_core PRIVATE SFML::Audio)

# --- engine_input: actions, remappable key bindings (client only)
add_library(engine_input
    engine/input/src/KeyBindings.cpp
    engine/input/src/InputManager.cpp
)
target_include_directories(engine_input PUBLIC engine/input/include)
target_link_libraries(engine_input PUBLIC engine_core SFML::Window)

# --- engine_ui: widgets for menus and settings (client only)
# add_library(engine_ui
#     engine/ui/src/Widget.cpp
#     engine/ui/src/Button.cpp
#     engine/ui/src/Label.cpp
#     engine/ui/src/Slider.cpp
# )
# target_include_directories(engine_ui PUBLIC engine/ui/include)
# target_link_libraries(engine_ui PUBLIC engine_render engine_input)

# ---------------------------------------------------------------------------
# Game: R-Type, built on the engine.
# ---------------------------------------------------------------------------

# --- rtype_shared: components, protocol, game data (client + server)
# Header-only for now: becomes a real library once its first .cpp is written.
add_library(rtype_shared INTERFACE)
target_include_directories(rtype_shared INTERFACE game/shared/include game/shared/include/components)
target_link_libraries(rtype_shared INTERFACE engine_core engine_ecs)

# Planned:
# add_library(rtype_shared
#     game/shared/src/protocol/Codec.cpp
#     game/shared/src/data/EnemyDefs.cpp
#     game/shared/src/data/WaveDefs.cpp
#     game/shared/src/data/GameRules.cpp
# )
# target_include_directories(rtype_shared PUBLIC game/shared/include)
# target_link_libraries(rtype_shared
#     PUBLIC engine_core engine_ecs engine_physics engine_net
#     PRIVATE nlohmann_json::nlohmann_json)

# --- r-type_server [p.1][p.9]: authoritative server
# For now: main.cpp only, linked directly to Asio. Once rtype_shared exists,
# link it instead and uncomment the files below as they are written.
add_executable(r-type_server
    game/server/src/main.cpp
)
target_link_libraries(r-type_server PRIVATE asio::asio Threads::Threads)
target_compile_definitions(r-type_server PRIVATE ASIO_STANDALONE)
if(WIN32)
    target_compile_definitions(r-type_server PRIVATE _WIN32_WINNT=0x0A00)
endif()

# add_executable(r-type_server
#     game/server/src/main.cpp
#     game/server/src/Server.cpp
#     game/server/src/Room.cpp
#     game/server/src/GameLoop.cpp
#     game/server/src/PacketHandler.cpp
#     game/server/src/EntityFactory.cpp
#     game/server/src/systems/InputSystem.cpp
#     game/server/src/systems/AISystem.cpp
#     game/server/src/systems/ShootingSystem.cpp
#     game/server/src/systems/DamageSystem.cpp
#     game/server/src/systems/DeathSystem.cpp
#     game/server/src/systems/SpawnSystem.cpp
#     game/server/src/systems/BoundarySystem.cpp
#     game/server/src/systems/SnapshotSystem.cpp
# )
# target_include_directories(r-type_server PRIVATE game/server/include)
# target_link_libraries(r-type_server PRIVATE rtype_shared)

# --- r-type_client [p.1][p.10]: display, input, sound
# Planned: NetworkClient, SceneManager, Settings, scenes/*, NetworkSync, Interpolation,
# Animation, Audio and Hud systems, accessibility/VisualSoundCues
add_executable(r-type_client
    game/client/src/main.cpp
    game/client/src/Client.cpp
    game/client/src/systems/MovementSystem.cpp
    game/client/src/systems/PlayerControlSystem.cpp
    game/client/src/systems/ScreenClampSystem.cpp
    game/client/src/systems/StarfieldSystem.cpp
    game/client/src/systems/RenderSystem.cpp
)
target_include_directories(r-type_client PRIVATE
    game/client/include
    game/client/include/components
    game/client/include/systems)
target_link_libraries(r-type_client PRIVATE rtype_shared engine_render engine_input)
add_custom_command(TARGET r-type_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:r-type_client>/assets)
