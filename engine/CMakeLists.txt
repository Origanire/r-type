cmake_minimum_required(VERSION 3.21)

# Standalone engine: builds the engine_* libraries, the generic launchers and engine_add_game().
# It knows nothing about any game; a game is a folder of JSON data, Python scripts and assets.
if(NOT PROJECT_NAME)
    project(Engine VERSION 0.1.0 LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
if(CMAKE_CONFIGURATION_TYPES)
    set(ENGINE_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>" CACHE INTERNAL "")
else()
    set(ENGINE_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" CACHE INTERNAL "")
endif()
set(ENGINE_APPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/apps/src" CACHE INTERNAL "")

# ---------------------------------------------------------------------------
# Dependencies (vcpkg.json). Check the exact target names printed by vcpkg.
# ---------------------------------------------------------------------------

find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
# Compile-time modularity: each switch removes a group of engine modules and its dependencies.
option(ENGINE_BUILD_CLIENT "Build the client modules (render, audio, input, client) and r-type_client" ON)
option(ENGINE_BUILD_SERVER "Build the server modules (scripting, simulation) and r-type_server" ON)
if(ENGINE_BUILD_CLIENT)
    find_package(SFML 3 COMPONENTS Graphics Window System Audio CONFIG REQUIRED)
endif()
if(ENGINE_BUILD_SERVER)
    find_package(Python3 COMPONENTS Interpreter Development.Embed REQUIRED)
    set(PYBIND11_FINDPYTHON ON)
    find_package(pybind11 CONFIG REQUIRED)
endif()

# ---------------------------------------------------------------------------
# Engine: knows nothing about R-Type. One module = one library [p.16].
# ---------------------------------------------------------------------------

# --- engine_core: errors, clock, logger, event bus, thread-safe queue, config, paths
# Planned: Logger.cpp, Random.cpp, Config.cpp, Paths.cpp
add_library(engine_core
    core/src/EngineError.cpp
    core/src/Clock.cpp
    core/src/FixedTimestep.cpp
    core/src/Color.cpp
    core/src/Vector2.cpp
    core/src/IntRect.cpp
    core/src/TextFormat.cpp
    core/src/CommandLine.cpp
    core/src/AssetPath.cpp
    core/src/PortOption.cpp
    core/src/UiInput.cpp
    core/src/ProcessMemory.cpp
)
target_include_directories(engine_core PUBLIC core/include)
if(WIN32)
    target_link_libraries(engine_core PRIVATE psapi)
endif()
# Uncomment once Config.cpp exists
# target_link_libraries(engine_core PRIVATE nlohmann_json::nlohmann_json)

# --- engine_physics: bounding boxes and collision detection (pure geometry)
add_library(engine_physics
    physics/src/Aabb.cpp
    physics/src/Collision.cpp
    physics/src/Body.cpp
    physics/src/CollisionPair.cpp
)
target_include_directories(engine_physics PUBLIC physics/include)

# --- engine_ecs: entities, registry, components and systems
# The render and audio interfaces used here (IRenderer, IAudio) contain no SFML: engine_ecs never links SFML.
add_library(engine_ecs
    ecs/src/Entity.cpp
    ecs/src/EntityManager.cpp
    ecs/src/Registry.cpp
    ecs/src/SystemManager.cpp
    ecs/src/UiStore.cpp
    ecs/src/WorldValues.cpp
    ecs/components/src/Script.cpp
    ecs/components/src/Attachment.cpp
    ecs/components/src/HitFlash.cpp
    ecs/components/src/Rotation.cpp
    ecs/components/src/PlayerValues.cpp
    ecs/components/src/Tilemap.cpp
    ecs/components/src/ParticleEmitter.cpp
    ecs/components/src/Lifetime.cpp
    ecs/components/src/MusicWhen.cpp
    ecs/components/src/Choice.cpp
    ecs/components/src/Gravity.cpp
    ecs/components/src/Bounce.cpp
    ecs/components/src/Transform.cpp
    ecs/components/src/Velocity.cpp
    ecs/components/src/Collider.cpp
    ecs/components/src/Bounded.cpp
    ecs/components/src/Health.cpp
    ecs/components/src/ContactDamage.cpp
    ecs/components/src/Invulnerable.cpp
    ecs/components/src/DespawnOffscreen.cpp
    ecs/components/src/PlayerControl.cpp
    ecs/components/src/Owner.cpp
    ecs/components/src/ScoreValue.cpp
    ecs/components/src/Sprite.cpp
    ecs/components/src/RectShape.cpp
    ecs/components/src/Parallax.cpp
    ecs/components/src/TiltAnimation.cpp
    ecs/components/src/Weapon.cpp
    ecs/components/src/Replicated.cpp
    ecs/components/src/SpriteAnimation.cpp
    ecs/components/src/SoundCue.cpp
    ecs/components/src/Text.cpp
    ecs/components/src/Anchor.cpp
    ecs/components/src/Player.cpp
    ecs/components/src/Lives.cpp
    ecs/components/src/Score.cpp
    ecs/components/src/Respawn.cpp
    ecs/components/src/Match.cpp
    ecs/components/src/DeathEffect.cpp
    ecs/components/src/PlayerIndex.cpp
    ecs/components/src/PlayerLabel.cpp
    ecs/components/src/StatusText.cpp
    ecs/components/src/Button.cpp
    ecs/components/src/Focusable.cpp
    ecs/components/src/TextField.cpp
    ecs/components/src/VisibleWhen.cpp
    ecs/components/src/BoundText.cpp
    ecs/components/src/RowData.cpp
    ecs/components/src/ListRowPart.cpp
    ecs/components/src/ListView.cpp
    ecs/components/src/ProgressBar.cpp
    ecs/components/src/FadeWhen.cpp
    ecs/components/src/ComponentRegistration.cpp
    ecs/systems/src/CollisionEvent.cpp
    ecs/systems/src/DeathEvent.cpp
    ecs/systems/src/MovementSystem.cpp
    ecs/systems/src/BoundsSystem.cpp
    ecs/systems/src/CollisionSystem.cpp
    ecs/systems/src/PlayerControlSystem.cpp
    ecs/systems/src/InvulnerabilitySystem.cpp
    ecs/systems/src/DamageSystem.cpp
    ecs/systems/src/OffscreenSystem.cpp
    ecs/systems/src/CleanupSystem.cpp
    ecs/systems/src/ParallaxSystem.cpp
    ecs/systems/src/TiltAnimationSystem.cpp
    ecs/systems/src/RenderSystem.cpp
    ecs/systems/src/WeaponSystem.cpp
    ecs/systems/src/SpriteAnimationSystem.cpp
    ecs/systems/src/AnchorSystem.cpp
    ecs/systems/src/AudioSystem.cpp
    ecs/systems/src/MatchSystem.cpp
    ecs/systems/src/RespawnSystem.cpp
    ecs/systems/src/LivesSystem.cpp
    ecs/systems/src/ScoreSystem.cpp
    ecs/systems/src/DeathEffectSystem.cpp
    ecs/systems/src/PlayerLabelSystem.cpp
    ecs/systems/src/StatusTextSystem.cpp
    ecs/systems/src/UiActionEvent.cpp
    ecs/systems/src/UiFocus.cpp
    ecs/systems/src/PointerSystem.cpp
    ecs/systems/src/FocusNavigationSystem.cpp
    ecs/systems/src/ButtonStyleSystem.cpp
    ecs/systems/src/TextFieldSystem.cpp
    ecs/systems/src/BoundTextSystem.cpp
    ecs/systems/src/VisibilitySystem.cpp
    ecs/systems/src/ListViewSystem.cpp
    ecs/systems/src/GameplaySystems.cpp
    ecs/systems/src/SoundEvent.cpp
    ecs/systems/src/ConsoleEvent.cpp
    ecs/systems/src/ProgressBarSystem.cpp
    ecs/systems/src/FadeSystem.cpp
)
target_include_directories(engine_ecs PUBLIC
    ecs/include
    ecs/components/include
    ecs/systems/include
    render/include
    audio/include)
target_link_libraries(engine_ecs PUBLIC engine_core engine_physics)

# --- engine_scene: JSON data, component and system factories, prefabs and scenes (nlohmann-json hidden inside)
add_library(engine_scene
    scene/src/DataNode.cpp
    scene/src/DataNodeImpl.cpp
    scene/src/DataReaders.cpp
    scene/src/LayerTable.cpp
    scene/src/ComponentFactory.cpp
    scene/src/EngineComponentLoaders.cpp
    scene/src/PrefabLibrary.cpp
    scene/src/SystemContext.cpp
    scene/src/SystemFactory.cpp
    scene/src/EngineSystemBuilders.cpp
    scene/src/SceneBuilder.cpp
    scene/src/WorldValueTable.cpp
)
target_include_directories(engine_scene PUBLIC scene/include PRIVATE scene/internal)
target_link_libraries(engine_scene PUBLIC engine_core engine_ecs PRIVATE nlohmann_json::nlohmann_json)

# --- engine_net: serialization, UDP client/server (Asio hidden inside) [p.9][p.11]
add_library(engine_net
    net/src/ByteReader.cpp
    net/src/ByteWriter.cpp
    net/src/Endpoint.cpp
    net/src/Datagram.cpp
    net/src/UdpTransport.cpp
    net/src/UdpTransportImpl.cpp
)
target_include_directories(engine_net PUBLIC net/include PRIVATE net/internal)
target_link_libraries(engine_net PUBLIC engine_core PRIVATE asio::asio Threads::Threads)
target_compile_definitions(engine_net PRIVATE ASIO_STANDALONE)
if(WIN32)
    target_compile_definitions(engine_net PRIVATE _WIN32_WINNT=0x0A00)
endif()

# --- engine_network: generic game protocol, game server and client connection on top of engine_net
add_library(engine_network
    network/src/NetworkSettings.cpp
    network/src/Codec.cpp
    network/src/Packet.cpp
    network/src/PlayerStatus.cpp
    network/src/EntityState.cpp
    network/src/ConnectMessage.cpp
    network/src/InputMessage.cpp
    network/src/HeartbeatMessage.cpp
    network/src/DisconnectMessage.cpp
    network/src/AcceptMessage.cpp
    network/src/RejectMessage.cpp
    network/src/SnapshotMessage.cpp
    network/src/HeartbeatAckMessage.cpp
    network/src/EntityDestroyedMessage.cpp
    network/src/PlayerJoinedMessage.cpp
    network/src/PlayerKilledMessage.cpp
    network/src/PlayerLeftMessage.cpp
    network/src/ServerShutdownMessage.cpp
    network/src/ListRoomsMessage.cpp
    network/src/CreateRoomMessage.cpp
    network/src/JoinRoomMessage.cpp
    network/src/LeaveRoomMessage.cpp
    network/src/StartGameMessage.cpp
    network/src/KickPlayerMessage.cpp
    network/src/DeleteRoomMessage.cpp
    network/src/RoomListMessage.cpp
    network/src/RoomJoinedMessage.cpp
    network/src/RoomRejectedMessage.cpp
    network/src/RoomUpdateMessage.cpp
    network/src/GameStartedMessage.cpp
    network/src/KickedMessage.cpp
    network/src/RoomClosedMessage.cpp
    network/src/PlaySoundMessage.cpp
    network/src/ConsoleCommandMessage.cpp
    network/src/ConsoleTextMessage.cpp
    network/src/TextRules.cpp
    network/src/RoomInfo.cpp
    network/src/GameRoom.cpp
    network/src/ClientSession.cpp
    network/src/GameServer.cpp
    network/src/ServerRunner.cpp
    network/src/ClientConnection.cpp
)
target_include_directories(engine_network PUBLIC network/include)
target_link_libraries(engine_network PUBLIC engine_core engine_net engine_scene)

# --- engine_render: window, SFML renderer, textures, fonts, palettes (client only)
# Planned: Palette.cpp
if(ENGINE_BUILD_CLIENT)
add_library(engine_render
    render/src/SfmlRenderer.cpp
    render/src/Window.cpp
    render/src/TextureManager.cpp
    render/src/FontManager.cpp
)
target_include_directories(engine_render PUBLIC render/include)
target_link_libraries(engine_render PUBLIC engine_core SFML::Graphics)

# --- engine_audio: sounds and music (client only)
add_library(engine_audio
    audio/src/SfmlAudio.cpp
)
target_include_directories(engine_audio PUBLIC audio/include)
target_link_libraries(engine_audio PUBLIC engine_core SFML::Audio)

# --- engine_input: actions, remappable key bindings (client only)
add_library(engine_input
    input/src/KeyBindings.cpp
    input/src/InputManager.cpp
)
target_include_directories(engine_input PUBLIC input/include)
target_link_libraries(engine_input PUBLIC engine_core SFML::Window)

# --- engine_client: window, resources, systems and scene of a game JSON file, fed by any transport (client only)
add_library(engine_client
    client/src/GameView.cpp
    client/src/PlayerSettings.cpp
    client/src/SceneManager.cpp
    client/src/DebugMetrics.cpp
    client/src/DevConsole.cpp
    client/src/OnlineClient.cpp
)
target_include_directories(engine_client PUBLIC client/include)
target_link_libraries(engine_client PUBLIC engine_scene engine_network engine_render engine_audio engine_input)

endif()

# --- engine_ui: widgets for menus and settings (client only)
# add_library(engine_ui
#     ui/src/Widget.cpp
#     ui/src/Button.cpp
#     ui/src/Label.cpp
#     ui/src/Slider.cpp
# )
# target_include_directories(engine_ui PUBLIC ui/include)
# target_link_libraries(engine_ui PUBLIC engine_render engine_input)

if(ENGINE_BUILD_SERVER)
# --- engine_scripting: runs the Python behaviour scripts named by Script components (server only)
add_library(engine_scripting
    scripting/src/ScriptEngine.cpp
    scripting/src/ScriptEngineImpl.cpp
    scripting/src/ScriptWorld.cpp
    scripting/src/ScriptEntity.cpp
    scripting/src/ScriptRecord.cpp
    scripting/src/ScriptSystem.cpp
    scripting/src/ScriptSystemImpl.cpp
    scripting/src/ScriptBuilder.cpp
)
target_include_directories(engine_scripting PUBLIC scripting/include PRIVATE scripting/internal)
target_link_libraries(engine_scripting PUBLIC engine_scene PRIVATE pybind11::embed)
set_target_properties(engine_scripting PROPERTIES CXX_VISIBILITY_PRESET hidden)

# --- engine_simulation: authoritative game world built from a game JSON file (no rendering)
add_library(engine_simulation
    simulation/src/Simulation.cpp
    simulation/src/SimulationFactory.cpp
)
target_include_directories(engine_simulation PUBLIC simulation/include)
target_link_libraries(engine_simulation PUBLIC engine_scene engine_network engine_scripting)

endif()


if(ENGINE_BUILD_SERVER)
# --- engine_python_runtime: copies the Python standard library next to the server so it runs without a system Python
set(ENGINE_PYTHON_PREFIX "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
if(WIN32)
    set(ENGINE_PYTHON_STDLIB "${ENGINE_PYTHON_PREFIX}/tools/python3/Lib")
    set(ENGINE_PYTHON_TARGET "python/Lib")
else()
    set(ENGINE_PYTHON_STDLIB "${ENGINE_PYTHON_PREFIX}/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
    set(ENGINE_PYTHON_TARGET "python/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
endif()
add_custom_target(engine_python_runtime ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ENGINE_PYTHON_STDLIB} ${ENGINE_OUTPUT_DIR}/${ENGINE_PYTHON_TARGET})
if(WIN32)
    add_custom_command(TARGET engine_python_runtime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ENGINE_PYTHON_PREFIX}/tools/python3/DLLs ${ENGINE_OUTPUT_DIR}/python/DLLs)
endif()

endif()

# engine_add_game(<name> CLIENT <exe> SERVER <exe> DATA <dir> SCRIPTS <dir> ASSETS <dir>)
# Builds the generic launchers under the game's executable names and copies its content next to them.
function(engine_add_game name)
    cmake_parse_arguments(GAME "" "CLIENT;SERVER;DATA;SCRIPTS;ASSETS" "" ${ARGN})
    add_custom_target(${name}_content ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${GAME_ASSETS} ${ENGINE_OUTPUT_DIR}/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${GAME_DATA} ${ENGINE_OUTPUT_DIR}/data
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${GAME_SCRIPTS} ${ENGINE_OUTPUT_DIR}/scripts)
    if(ENGINE_BUILD_SERVER AND GAME_SERVER)
        add_executable(${GAME_SERVER} ${ENGINE_APPS_DIR}/ServerMain.cpp)
        target_compile_definitions(${GAME_SERVER} PRIVATE ENGINE_PROGRAM_NAME="${GAME_SERVER}" ENGINE_DEFAULT_GAME="${name}")
        target_link_libraries(${GAME_SERVER} PRIVATE engine_simulation)
        add_dependencies(${GAME_SERVER} ${name}_content engine_python_runtime)
        set_target_properties(${GAME_SERVER} PROPERTIES ENABLE_EXPORTS ON)
    endif()
    if(ENGINE_BUILD_CLIENT AND GAME_CLIENT)
        add_executable(${GAME_CLIENT} ${ENGINE_APPS_DIR}/ClientMain.cpp)
        target_compile_definitions(${GAME_CLIENT} PRIVATE ENGINE_PROGRAM_NAME="${GAME_CLIENT}" ENGINE_DEFAULT_GAME="${name}")
        target_link_libraries(${GAME_CLIENT} PRIVATE engine_client)
        add_dependencies(${GAME_CLIENT} ${name}_content)
    endif()
endfunction()
