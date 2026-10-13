cmake_minimum_required(VERSION 3.21)

# Select the toolchain before project() initializes the compiler. Also repair
# the path produced by the README command when VCPKG_ROOT is unset.
if(NOT CMAKE_TOOLCHAIN_FILE OR
   CMAKE_TOOLCHAIN_FILE STREQUAL "/scripts/buildsystems/vcpkg.cmake")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE
            "${CMAKE_CURRENT_SOURCE_DIR}/.vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE FILEPATH "vcpkg toolchain" FORCE)
        message(STATUS "Using project-local vcpkg: ${CMAKE_TOOLCHAIN_FILE}")
    endif()
endif()

if(IS_ABSOLUTE "${CMAKE_TOOLCHAIN_FILE}" AND NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    message(FATAL_ERROR
        "CMAKE_TOOLCHAIN_FILE does not exist: ${CMAKE_TOOLCHAIN_FILE}. "
        "Install vcpkg and set VCPKG_ROOT to its directory (see README.md). "
        "Clear the CMake cache before configuring again.")
endif()

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
add_library(engine_net engine/net/src/UdpTransport.cpp)
target_include_directories(engine_net PUBLIC engine/net/include)
target_link_libraries(engine_net PRIVATE asio::asio Threads::Threads)
target_compile_definitions(engine_net PRIVATE ASIO_STANDALONE)
if(WIN32)
    target_compile_definitions(engine_net PRIVATE _WIN32_WINNT=0x0A00)
endif()
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
add_library(rtype_shared game/shared/src/protocol/Codec.cpp game/shared/src/PortOption.cpp)
target_include_directories(rtype_shared PUBLIC game/shared/include game/shared/include/components)
target_link_libraries(rtype_shared PUBLIC engine_core engine_ecs engine_net)

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
# Authoritative movement simulation; rendering/input remain client-only.
add_executable(r-type_server
    game/server/src/main.cpp
    game/server/src/Server.cpp
)
target_include_directories(r-type_server PRIVATE game/server/include)
target_link_libraries(r-type_server PRIVATE rtype_shared)

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
    game/client/src/NetworkClient.cpp
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

include(CTest)
if(BUILD_TESTING)
    add_executable(protocol_tests tests/ProtocolTests.cpp)
    target_link_libraries(protocol_tests PRIVATE rtype_shared)
    add_test(NAME protocol COMMAND protocol_tests)

    add_executable(network_client_tests tests/NetworkClientTests.cpp game/client/src/NetworkClient.cpp)
    target_include_directories(network_client_tests PRIVATE game/client/include)
    target_link_libraries(network_client_tests PRIVATE rtype_shared)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(NAME udp_integration COMMAND ${Python3_EXECUTABLE}
            ${PROJECT_SOURCE_DIR}/tests/network_integration.py
            $<TARGET_FILE:r-type_server> $<TARGET_FILE:network_client_tests>)
        set_tests_properties(udp_integration PROPERTIES TIMEOUT 25)
    endif()
endif()
