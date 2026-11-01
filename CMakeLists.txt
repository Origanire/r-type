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

# --- engine_physics: bounding boxes and collision detection (pure geometry)
add_library(engine_physics
    engine/physics/src/Aabb.cpp
    engine/physics/src/Collision.cpp
)
target_include_directories(engine_physics PUBLIC engine/physics/include)

# --- engine_ecs: entities, registry, components and systems
# The render headers used here (IRenderer, RenderTypes) contain no SFML: engine_ecs never links SFML.
add_library(engine_ecs
    engine/ecs/src/Entity.cpp
    engine/ecs/src/EntityManager.cpp
    engine/ecs/src/Registry.cpp
    engine/ecs/src/SystemManager.cpp
    engine/ecs/components/src/Transform.cpp
    engine/ecs/components/src/Velocity.cpp
    engine/ecs/components/src/Collider.cpp
    engine/ecs/components/src/Bounded.cpp
    engine/ecs/components/src/Health.cpp
    engine/ecs/components/src/ContactDamage.cpp
    engine/ecs/components/src/Invulnerable.cpp
    engine/ecs/components/src/MovementPattern.cpp
    engine/ecs/components/src/DespawnOffscreen.cpp
    engine/ecs/components/src/PlayerControl.cpp
    engine/ecs/components/src/Owner.cpp
    engine/ecs/components/src/ScoreValue.cpp
    engine/ecs/components/src/Sprite.cpp
    engine/ecs/components/src/RectShape.cpp
    engine/ecs/components/src/Parallax.cpp
    engine/ecs/components/src/TiltAnimation.cpp
    engine/ecs/components/src/ComponentRegistration.cpp
    engine/ecs/systems/src/CollisionEvent.cpp
    engine/ecs/systems/src/DeathEvent.cpp
    engine/ecs/systems/src/MovementSystem.cpp
    engine/ecs/systems/src/BoundsSystem.cpp
    engine/ecs/systems/src/CollisionSystem.cpp
    engine/ecs/systems/src/PlayerControlSystem.cpp
    engine/ecs/systems/src/MovementPatternSystem.cpp
    engine/ecs/systems/src/InvulnerabilitySystem.cpp
    engine/ecs/systems/src/DamageSystem.cpp
    engine/ecs/systems/src/OffscreenSystem.cpp
    engine/ecs/systems/src/CleanupSystem.cpp
    engine/ecs/systems/src/ParallaxSystem.cpp
    engine/ecs/systems/src/TiltAnimationSystem.cpp
    engine/ecs/systems/src/RenderSystem.cpp
)
target_include_directories(engine_ecs PUBLIC
    engine/ecs/include
    engine/ecs/components/include
    engine/ecs/systems/include
    engine/render/include)
target_link_libraries(engine_ecs PUBLIC engine_core engine_physics)

# --- engine_net: serialization, UDP client/server (Asio hidden inside) [p.9][p.11]
add_library(engine_net
    engine/net/src/ByteReader.cpp
    engine/net/src/ByteWriter.cpp
    engine/net/src/UdpTransport.cpp
)
target_include_directories(engine_net PUBLIC engine/net/include)
target_link_libraries(engine_net PUBLIC engine_core PRIVATE asio::asio Threads::Threads)
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
add_library(rtype_shared
    game/shared/src/protocol/Codec.cpp
    game/shared/src/PortOption.cpp
)
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

# --- r-type_server [p.1][p.9]: authoritative server (never links render, audio, input or ui)
add_executable(r-type_server
    game/server/src/main.cpp
    game/server/src/Server.cpp
    game/server/src/Room.cpp
    game/server/src/EntityFactory.cpp
    game/server/src/GameEvents.cpp
    game/server/src/systems/InputSystem.cpp
    game/server/src/systems/ShootingSystem.cpp
    game/server/src/systems/SpawnSystem.cpp
    game/server/src/systems/EnemyMovementSystem.cpp
    game/server/src/systems/InvulnerabilitySystem.cpp
    game/server/src/systems/MovementSystem.cpp
    game/server/src/systems/BoundarySystem.cpp
    game/server/src/systems/CollisionSystem.cpp
    game/server/src/systems/OffscreenSystem.cpp
    game/server/src/Snapshot.cpp
)
target_include_directories(r-type_server PRIVATE
    game/server/include
    game/server/include/components
    game/server/include/systems)
target_link_libraries(r-type_server PRIVATE rtype_shared engine_physics)

# --- r-type_client [p.1][p.10]: display, input, sound
# Planned: SceneManager, Settings, scenes/*, NetworkSync, Interpolation,
# Animation, Audio and Hud systems, accessibility/VisualSoundCues
add_executable(r-type_client
    game/client/src/main.cpp
    game/client/src/Client.cpp
    game/client/src/NetworkClient.cpp
    game/client/src/SpriteSheet.cpp
    game/client/src/systems/StarfieldSystem.cpp
    game/client/src/systems/ShipAnimationSystem.cpp
    game/client/src/systems/RenderSystem.cpp
)
target_include_directories(r-type_client PRIVATE
    game/client/include
    game/client/include/components
    game/client/include/systems)
target_link_libraries(r-type_client PRIVATE rtype_shared engine_render engine_input)
add_custom_command(TARGET r-type_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:r-type_client>/assets)
