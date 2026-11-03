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
find_package(SFML 3 COMPONENTS Graphics Window System Audio CONFIG REQUIRED)

# ---------------------------------------------------------------------------
# Engine: knows nothing about R-Type. One module = one library [p.16].
# ---------------------------------------------------------------------------

# --- engine_core: errors, clock, logger, event bus, thread-safe queue, config, paths
# Planned: Logger.cpp, Random.cpp, Config.cpp, Paths.cpp
add_library(engine_core
    engine/core/src/EngineError.cpp
    engine/core/src/Clock.cpp
    engine/core/src/FixedTimestep.cpp
    engine/core/src/Color.cpp
    engine/core/src/Vector2.cpp
    engine/core/src/IntRect.cpp
    engine/core/src/TextFormat.cpp
    engine/core/src/CommandLine.cpp
    engine/core/src/AssetPath.cpp
    engine/core/src/PortOption.cpp
)
target_include_directories(engine_core PUBLIC engine/core/include)
# Uncomment once Config.cpp exists
# target_link_libraries(engine_core PRIVATE nlohmann_json::nlohmann_json)

# --- engine_physics: bounding boxes and collision detection (pure geometry)
add_library(engine_physics
    engine/physics/src/Aabb.cpp
    engine/physics/src/Collision.cpp
    engine/physics/src/Body.cpp
    engine/physics/src/CollisionPair.cpp
)
target_include_directories(engine_physics PUBLIC engine/physics/include)

# --- engine_ecs: entities, registry, components and systems
# The render and audio interfaces used here (IRenderer, IAudio) contain no SFML: engine_ecs never links SFML.
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
    engine/ecs/components/src/Weapon.cpp
    engine/ecs/components/src/SpawnEntry.cpp
    engine/ecs/components/src/Spawner.cpp
    engine/ecs/components/src/Replicated.cpp
    engine/ecs/components/src/AutoFire.cpp
    engine/ecs/components/src/SpriteAnimation.cpp
    engine/ecs/components/src/SoundCue.cpp
    engine/ecs/components/src/Text.cpp
    engine/ecs/components/src/Anchor.cpp
    engine/ecs/components/src/Player.cpp
    engine/ecs/components/src/Lives.cpp
    engine/ecs/components/src/Score.cpp
    engine/ecs/components/src/Respawn.cpp
    engine/ecs/components/src/Match.cpp
    engine/ecs/components/src/DeathEffect.cpp
    engine/ecs/components/src/PlayerIndex.cpp
    engine/ecs/components/src/PlayerLabel.cpp
    engine/ecs/components/src/StatusText.cpp
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
    engine/ecs/systems/src/WeaponSystem.cpp
    engine/ecs/systems/src/SpawnerSystem.cpp
    engine/ecs/systems/src/AutoFireSystem.cpp
    engine/ecs/systems/src/SpriteAnimationSystem.cpp
    engine/ecs/systems/src/AnchorSystem.cpp
    engine/ecs/systems/src/AudioSystem.cpp
    engine/ecs/systems/src/MatchSystem.cpp
    engine/ecs/systems/src/RespawnSystem.cpp
    engine/ecs/systems/src/LivesSystem.cpp
    engine/ecs/systems/src/ScoreSystem.cpp
    engine/ecs/systems/src/DeathEffectSystem.cpp
    engine/ecs/systems/src/PlayerLabelSystem.cpp
    engine/ecs/systems/src/StatusTextSystem.cpp
)
target_include_directories(engine_ecs PUBLIC
    engine/ecs/include
    engine/ecs/components/include
    engine/ecs/systems/include
    engine/render/include
    engine/audio/include)
target_link_libraries(engine_ecs PUBLIC engine_core engine_physics)

# --- engine_scene: JSON data, component and system factories, prefabs and scenes (nlohmann-json hidden inside)
add_library(engine_scene
    engine/scene/src/DataNode.cpp
    engine/scene/src/DataNodeImpl.cpp
    engine/scene/src/DataReaders.cpp
    engine/scene/src/LayerTable.cpp
    engine/scene/src/ComponentFactory.cpp
    engine/scene/src/EngineComponentLoaders.cpp
    engine/scene/src/PrefabLibrary.cpp
    engine/scene/src/SystemContext.cpp
    engine/scene/src/SystemFactory.cpp
    engine/scene/src/EngineSystemBuilders.cpp
    engine/scene/src/SceneBuilder.cpp
)
target_include_directories(engine_scene PUBLIC engine/scene/include PRIVATE engine/scene/internal)
target_link_libraries(engine_scene PUBLIC engine_core engine_ecs PRIVATE nlohmann_json::nlohmann_json)

# --- engine_net: serialization, UDP client/server (Asio hidden inside) [p.9][p.11]
add_library(engine_net
    engine/net/src/ByteReader.cpp
    engine/net/src/ByteWriter.cpp
    engine/net/src/Endpoint.cpp
    engine/net/src/Datagram.cpp
    engine/net/src/UdpTransport.cpp
    engine/net/src/UdpTransportImpl.cpp
)
target_include_directories(engine_net PUBLIC engine/net/include PRIVATE engine/net/internal)
target_link_libraries(engine_net PUBLIC engine_core PRIVATE asio::asio Threads::Threads)
target_compile_definitions(engine_net PRIVATE ASIO_STANDALONE)
if(WIN32)
    target_compile_definitions(engine_net PRIVATE _WIN32_WINNT=0x0A00)
endif()

# --- engine_network: generic game protocol, game server and client connection on top of engine_net
add_library(engine_network
    engine/network/src/NetworkSettings.cpp
    engine/network/src/Codec.cpp
    engine/network/src/Packet.cpp
    engine/network/src/PlayerStatus.cpp
    engine/network/src/EntityState.cpp
    engine/network/src/ConnectMessage.cpp
    engine/network/src/InputMessage.cpp
    engine/network/src/HeartbeatMessage.cpp
    engine/network/src/DisconnectMessage.cpp
    engine/network/src/AcceptMessage.cpp
    engine/network/src/RejectMessage.cpp
    engine/network/src/SnapshotMessage.cpp
    engine/network/src/HeartbeatAckMessage.cpp
    engine/network/src/EntityDestroyedMessage.cpp
    engine/network/src/PlayerJoinedMessage.cpp
    engine/network/src/PlayerKilledMessage.cpp
    engine/network/src/PlayerLeftMessage.cpp
    engine/network/src/ServerShutdownMessage.cpp
    engine/network/src/ClientSession.cpp
    engine/network/src/GameServer.cpp
    engine/network/src/ServerRunner.cpp
    engine/network/src/ClientConnection.cpp
)
target_include_directories(engine_network PUBLIC engine/network/include)
target_link_libraries(engine_network PUBLIC engine_core engine_net engine_scene)

# --- engine_render: window, SFML renderer, textures, fonts, palettes (client only)
# Planned: Palette.cpp
add_library(engine_render
    engine/render/src/SfmlRenderer.cpp
    engine/render/src/Window.cpp
    engine/render/src/TextureManager.cpp
    engine/render/src/FontManager.cpp
)
target_include_directories(engine_render PUBLIC engine/render/include)
target_link_libraries(engine_render PUBLIC engine_core SFML::Graphics)

# --- engine_audio: sounds and music (client only)
add_library(engine_audio
    engine/audio/src/SfmlAudio.cpp
)
target_include_directories(engine_audio PUBLIC engine/audio/include)
target_link_libraries(engine_audio PUBLIC engine_core SFML::Audio)

# --- engine_input: actions, remappable key bindings (client only)
add_library(engine_input
    engine/input/src/KeyBindings.cpp
    engine/input/src/InputManager.cpp
)
target_include_directories(engine_input PUBLIC engine/input/include)
target_link_libraries(engine_input PUBLIC engine_core SFML::Window)

# --- engine_client: window, resources, systems and scene of a game JSON file, fed by any transport (client only)
add_library(engine_client
    engine/client/src/GameView.cpp
    engine/client/src/PlayerSettings.cpp
    engine/client/src/OnlineClient.cpp
)
target_include_directories(engine_client PUBLIC engine/client/include)
target_link_libraries(engine_client PUBLIC engine_scene engine_network engine_render engine_audio engine_input)

# --- engine_ui: widgets for menus and settings (client only)
# add_library(engine_ui
#     engine/ui/src/Widget.cpp
#     engine/ui/src/Button.cpp
#     engine/ui/src/Label.cpp
#     engine/ui/src/Slider.cpp
# )
# target_include_directories(engine_ui PUBLIC engine/ui/include)
# target_link_libraries(engine_ui PUBLIC engine_render engine_input)

# --- engine_simulation: authoritative game world built from a game JSON file (no rendering)
add_library(engine_simulation
    engine/simulation/src/Simulation.cpp
)
target_include_directories(engine_simulation PUBLIC engine/simulation/include)
target_link_libraries(engine_simulation PUBLIC engine_scene engine_network)

# ---------------------------------------------------------------------------
# Game: R-Type, described only by the JSON files of game/data.
# ---------------------------------------------------------------------------

# --- rtype_assets: copies assets/ (sprites) and game/data/ (R-Type JSON files) next to the executables
add_custom_target(rtype_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:r-type_client>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/game/data $<TARGET_FILE_DIR:r-type_client>/data)

# --- r-type_server [p.1][p.9]: authoritative server (never links render, audio, input or ui)
add_executable(r-type_server
    game/server/src/main.cpp
)
target_link_libraries(r-type_server PRIVATE engine_simulation)
add_dependencies(r-type_server rtype_assets)

# --- r-type_client [p.1][p.10]: display, input, sound
add_executable(r-type_client
    game/client/src/main.cpp
)
target_link_libraries(r-type_client PRIVATE engine_client)
add_dependencies(r-type_client rtype_assets)
