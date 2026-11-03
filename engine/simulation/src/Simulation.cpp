#include "Simulation.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <utility>

#include "ComponentRegistration.hpp"
#include "Destroyed.hpp"
#include "EngineComponentLoaders.hpp"
#include "EngineSystemBuilders.hpp"
#include "Lives.hpp"
#include "MatchSystem.hpp"
#include "Player.hpp"
#include "PlayerControl.hpp"
#include "Protocol.hpp"
#include "Replicated.hpp"
#include "SceneBuilder.hpp"
#include "Score.hpp"
#include "SimulationError.hpp"
#include "Transform.hpp"

namespace engine::simulation {

Simulation::Simulation(
    const std::filesystem::path &dataDirectory, const std::string &game, const network::NetworkSettings &settings)
    : m_settings(settings), m_game(scene::DataNode::fromFile(dataDirectory / (game + "_game.json"))),
      m_prefabs(m_registry, m_components), m_context(m_registry, m_prefabs), m_cleanup(m_registry),
      m_playerPrefab(m_game.getString("playerPrefab")), m_events(), m_reportedPhase(ecs::Match::Phase::Waiting),
      m_nextNetworkId(1), m_tick(0)
{
    ecs::registerEngineComponents(m_registry);
    m_layers.load(m_game.child("layers"));
    scene::EngineComponentLoaders::registerAll(m_components, m_layers);
    scene::EngineSystemBuilders::registerAll(m_systemFactory);
    m_prefabs.load(scene::DataNode::fromFile(dataDirectory / m_game.child("files").getString("serverPrefabs")));
    if (!m_prefabs.has(m_playerPrefab))
        throw SimulationError("Simulation: unknown player prefab " + m_playerPrefab);
    m_context.setCanSpawn(std::bind_front(&Simulation::canSpawn, this));
    m_systemFactory.build(m_context, m_game.child("serverSystems"), m_systems);
    const scene::DataNode world = m_game.child("world");
    scene::SceneBuilder(m_registry, m_prefabs, m_components, {world.getFloat("width"), world.getFloat("height")})
        .build(m_game.child("serverScene"));
}

void Simulation::addPlayer(std::size_t slot)
{
    if (slot >= m_settings.maxPlayers() || playerEntity(slot))
        throw SimulationError("Simulation: player slot " + std::to_string(slot) + " is invalid or already used");
    const ecs::Entity player = m_prefabs.spawn(m_playerPrefab);
    m_registry.emplace<ecs::Player>(player, slot);
}

void Simulation::removePlayer(std::size_t slot)
{
    const auto player = playerEntity(slot);
    if (!player)
        return;
    if (const auto avatar = avatarOf(slot))
        m_registry.kill(*avatar);
    m_registry.kill(*player);
}

void Simulation::setInput(std::size_t slot, std::uint8_t buttons)
{
    const auto avatar = avatarOf(slot);
    if (!avatar || !m_registry.has<ecs::PlayerControl>(*avatar))
        return;
    ecs::PlayerControl &control = m_registry.get<ecs::PlayerControl>(*avatar);
    control.setDirection(buttons & network::Protocol::BUTTON_UP, buttons & network::Protocol::BUTTON_DOWN,
        buttons & network::Protocol::BUTTON_LEFT, buttons & network::Protocol::BUTTON_RIGHT);
    control.setFiring(buttons & network::Protocol::BUTTON_FIRE);
}

void Simulation::update(float dt)
{
    m_systems.update(dt);
    assignNetworkIds();
    collectDeaths();
    m_context.deaths().clear();
    m_context.collisions().clear();
    m_cleanup.update(dt);
    reportPhase();
    ++m_tick;
}

std::uint32_t Simulation::tick() const
{
    return m_tick;
}

network::SnapshotMessage Simulation::snapshot() const
{
    network::SnapshotMessage::Players players;
    players.reserve(m_settings.maxPlayers());
    for (std::size_t slot = 0; slot < m_settings.maxPlayers(); ++slot)
        players.push_back(statusOf(slot));
    return network::SnapshotMessage(m_tick, phaseCode(phase()), std::move(players), collectEntities());
}

std::vector<network::Packet> Simulation::takeEvents()
{
    return std::exchange(m_events, {});
}

ecs::Match::Phase Simulation::phase() const
{
    return ecs::MatchSystem::phaseOf(m_registry);
}

const ecs::Registry &Simulation::registry() const
{
    return m_registry;
}

std::optional<ecs::Entity> Simulation::playerEntity(std::size_t slot) const
{
    const auto &players = m_registry.getComponents<ecs::Player>();
    for (std::size_t i : m_registry.view<ecs::Player>()) {
        if (players.at(i).slot() == slot)
            return m_registry.entityAt(i);
    }
    return std::nullopt;
}

std::optional<ecs::Entity> Simulation::avatarOf(std::size_t slot) const
{
    const auto player = playerEntity(slot);
    if (!player)
        return std::nullopt;
    const auto avatar = m_registry.get<ecs::Player>(*player).avatar();
    if (!avatar || !m_registry.isAlive(*avatar) || m_registry.has<ecs::Destroyed>(*avatar))
        return std::nullopt;
    return avatar;
}

void Simulation::collectDeaths()
{
    const auto &players = m_registry.getComponents<ecs::Player>();
    for (const ecs::DeathEvent &death : m_context.deaths().events()) {
        const ecs::Entity victim = death.victim();
        const std::uint16_t networkId = networkIdOf(victim);
        if (networkId != 0)
            m_events.emplace_back(network::EntityDestroyedMessage(networkId));
        for (std::size_t i : m_registry.view<ecs::Player>()) {
            if (players.at(i).avatar() != victim)
                continue;
            const std::size_t slot = players.at(i).slot();
            m_events.emplace_back(network::PlayerKilledMessage(static_cast<std::uint8_t>(slot)));
            std::cout << "Player " << slot << " killed" << std::endl;
        }
    }
}

void Simulation::assignNetworkIds()
{
    auto &replicated = m_registry.getComponents<ecs::Replicated>();
    for (std::size_t i : m_registry.view<ecs::Replicated>()) {
        if (replicated.at(i).networkId() == 0)
            replicated.at(i).setNetworkId(nextNetworkId());
    }
}

void Simulation::reportPhase()
{
    const ecs::Match::Phase current = phase();
    if (current == m_reportedPhase)
        return;
    if (current == ecs::Match::Phase::Over)
        std::cout << "Game over" << std::endl;
    else if (current == ecs::Match::Phase::Playing)
        std::cout << "New round" << std::endl;
    m_reportedPhase = current;
}

bool Simulation::canSpawn() const
{
    return phase() == ecs::Match::Phase::Playing
        && m_registry.view<ecs::Replicated>().size() < m_settings.maxEntities();
}

network::PlayerStatus Simulation::statusOf(std::size_t slot) const
{
    const auto player = playerEntity(slot);
    if (!player)
        return network::PlayerStatus();
    const int lives = m_registry.has<ecs::Lives>(*player) ? m_registry.get<ecs::Lives>(*player).count() : 0;
    const std::uint32_t score = m_registry.has<ecs::Score>(*player) ? m_registry.get<ecs::Score>(*player).points() : 0;
    const auto avatar = avatarOf(slot);
    const std::uint16_t avatarId = avatar ? networkIdOf(*avatar) : 0;
    const std::uint8_t state = avatarId != 0 ? network::Protocol::SLOT_ALIVE : network::Protocol::SLOT_DEAD;
    return network::PlayerStatus(
        state, static_cast<std::uint8_t>(std::clamp(lives, 0, MAX_REPORTED_LIVES)), score, avatarId);
}

std::vector<network::EntityState> Simulation::collectEntities() const
{
    std::vector<network::EntityState> entities;
    std::vector<ecs::Entity> avatars;
    for (std::size_t slot = 0; slot < m_settings.maxPlayers(); ++slot) {
        const auto avatar = avatarOf(slot);
        if (!avatar)
            continue;
        avatars.push_back(*avatar);
        if (const auto described = describe(*avatar))
            entities.push_back(*described);
    }
    for (std::size_t i : m_registry.view<ecs::Transform, ecs::Replicated>()) {
        if (entities.size() >= m_settings.maxEntities())
            break;
        const ecs::Entity entity = m_registry.entityAt(i);
        if (std::find(avatars.begin(), avatars.end(), entity) != avatars.end())
            continue;
        if (const auto described = describe(entity))
            entities.push_back(*described);
    }
    return entities;
}

std::optional<network::EntityState> Simulation::describe(const ecs::Entity &entity) const
{
    if (!m_registry.has<ecs::Transform>(entity) || !m_registry.has<ecs::Replicated>(entity))
        return std::nullopt;
    const ecs::Replicated &identity = m_registry.get<ecs::Replicated>(entity);
    if (identity.networkId() == 0)
        return std::nullopt;
    const ecs::Transform &transform = m_registry.get<ecs::Transform>(entity);
    return network::EntityState(identity.networkId(), identity.kind(),
        static_cast<std::int16_t>(std::lround(transform.x())), static_cast<std::int16_t>(std::lround(transform.y())));
}

std::uint16_t Simulation::networkIdOf(const ecs::Entity &entity) const
{
    if (!m_registry.isAlive(entity) || !m_registry.has<ecs::Replicated>(entity))
        return 0;
    return m_registry.get<ecs::Replicated>(entity).networkId();
}

std::uint16_t Simulation::nextNetworkId()
{
    while (m_nextNetworkId == 0 || networkIdInUse(m_nextNetworkId))
        ++m_nextNetworkId;
    return m_nextNetworkId++;
}

bool Simulation::networkIdInUse(std::uint16_t networkId) const
{
    const auto &replicated = m_registry.getComponents<ecs::Replicated>();
    for (std::size_t i : m_registry.view<ecs::Replicated>()) {
        if (replicated.at(i).networkId() == networkId)
            return true;
    }
    return false;
}

std::uint8_t Simulation::phaseCode(ecs::Match::Phase phase)
{
    if (phase == ecs::Match::Phase::Playing)
        return network::Protocol::GAME_PLAYING;
    if (phase == ecs::Match::Phase::Over)
        return network::Protocol::GAME_OVER;
    return network::Protocol::GAME_WAITING;
}

} // namespace engine::simulation
