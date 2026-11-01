#include "Room.hpp"

#include <algorithm>
#include <iostream>
#include <memory>

#include "BoundsSystem.hpp"
#include "CollisionSystem.hpp"
#include "ComponentRegistration.hpp"
#include "DamageSystem.hpp"
#include "EngineComponentLoaders.hpp"
#include "GameError.hpp"
#include "Invulnerable.hpp"
#include "InvulnerabilitySystem.hpp"
#include "MovementPatternSystem.hpp"
#include "MovementSystem.hpp"
#include "OffscreenSystem.hpp"
#include "PlayerControl.hpp"
#include "PlayerControlSystem.hpp"
#include "Replicated.hpp"
#include "ScoreValue.hpp"
#include "Snapshot.hpp"
#include "SpawnerSystem.hpp"
#include "Transform.hpp"
#include "WeaponSystem.hpp"

namespace rtype::server {

using engine::ecs::Entity;

Room::Room(const std::filesystem::path &dataDirectory)
    : m_config(engine::scene::DataNode::fromFile(dataDirectory / "rtype_game.json")), m_rules(m_config.child("rules")),
      m_prefabs(m_registry, m_factory), m_cleanup(m_registry)
{
    engine::ecs::registerEngineComponents(m_registry);
    m_layers.load(m_config.child("layers"));
    engine::scene::addEngineComponentLoaders(m_factory, m_layers);
    m_prefabs.load(engine::scene::DataNode::fromFile(dataDirectory / "rtype_server_prefabs.json"));
    registerSystems();
    m_prefabs.spawn(m_rules.spawnerPrefab());
}

void Room::spawnPlayer(std::size_t slot)
{
    if (slot >= m_players.size() || m_players[slot])
        throw GameError("Room: invalid or occupied player slot");
    m_players[slot] = PlayerSlot(m_rules.startLives());
    if (m_gameState == protocol::GAME_WAITING)
        m_gameState = protocol::GAME_PLAYING;
    if (m_gameState == protocol::GAME_PLAYING)
        spawnShip(slot, false);
}

void Room::removePlayer(std::size_t slot)
{
    if (slot >= m_players.size() || !m_players[slot])
        return;
    if (m_ships[slot])
        m_registry.kill(*m_ships[slot]);
    m_ships[slot].reset();
    m_players[slot].reset();
    if (std::none_of(m_players.begin(), m_players.end(), [](const auto &player) { return player.has_value(); })) {
        clearWorld();
        m_gameState = protocol::GAME_WAITING;
    }
}

void Room::setButtons(std::size_t slot, std::uint8_t buttons)
{
    if (slot >= m_ships.size() || !m_ships[slot])
        return;
    auto &control = m_registry.get<engine::ecs::PlayerControl>(*m_ships[slot]);
    control.setDirection(buttons & protocol::BUTTON_UP, buttons & protocol::BUTTON_DOWN,
        buttons & protocol::BUTTON_LEFT, buttons & protocol::BUTTON_RIGHT);
    control.setFiring(buttons & protocol::BUTTON_FIRE);
}

void Room::update(float dt)
{
    updateRespawns(dt);
    m_systems.update(dt);
    assignNetworkIds();
    handleDeaths();
    updateGameState(dt);
    m_cleanup.update(dt);
    ++m_tick;
}

std::uint32_t Room::tick() const
{
    return m_tick;
}

std::uint8_t Room::gameState() const
{
    return m_gameState;
}

std::array<protocol::PlayerState, protocol::MAX_PLAYERS> Room::playerStates() const
{
    std::array<protocol::PlayerState, protocol::MAX_PLAYERS> states{};
    for (std::size_t slot = 0; slot < m_players.size(); ++slot) {
        if (!m_players[slot])
            continue;
        const std::uint16_t shipId = m_ships[slot] ? networkIdOf(*m_ships[slot]) : 0;
        const std::uint8_t state = shipId != 0 ? protocol::SLOT_ALIVE : protocol::SLOT_DEAD;
        states[slot] = {state, m_players[slot]->lives(), m_players[slot]->score(), shipId};
    }
    return states;
}

std::vector<protocol::NetworkEntity> Room::networkEntities() const
{
    return collectNetworkEntities(m_registry);
}

std::vector<protocol::Packet> Room::takeEvents()
{
    return m_events.take();
}

void Room::registerSystems()
{
    m_systems.add(std::make_unique<engine::ecs::PlayerControlSystem>(m_registry));
    m_systems.add(std::make_unique<engine::ecs::WeaponSystem>(
        m_registry, m_prefabs, [this] { return canSpawnReplicated(); }));
    m_systems.add(std::make_unique<engine::ecs::SpawnerSystem>(
        m_registry, m_prefabs, [this] { return m_gameState == protocol::GAME_PLAYING && canSpawnReplicated(); }));
    m_systems.add(std::make_unique<engine::ecs::MovementPatternSystem>(m_registry));
    m_systems.add(std::make_unique<engine::ecs::InvulnerabilitySystem>(m_registry));
    m_systems.add(std::make_unique<engine::ecs::MovementSystem>(m_registry));
    m_systems.add(std::make_unique<engine::ecs::BoundsSystem>(m_registry));
    m_systems.add(std::make_unique<engine::ecs::CollisionSystem>(m_registry, m_collisions));
    m_systems.add(std::make_unique<engine::ecs::DamageSystem>(m_registry, m_collisions, m_deaths));
    m_systems.add(std::make_unique<engine::ecs::OffscreenSystem>(m_registry));
}

void Room::updateRespawns(float dt)
{
    if (m_gameState != protocol::GAME_PLAYING)
        return;
    for (std::size_t slot = 0; slot < m_players.size(); ++slot) {
        if (!m_players[slot] || m_ships[slot] || m_players[slot]->lives() == 0)
            continue;
        m_players[slot]->elapseRespawn(dt);
        if (m_players[slot]->respawnTimer() <= 0.f)
            spawnShip(slot, true);
    }
}

void Room::updateGameState(float dt)
{
    if (m_gameState == protocol::GAME_PLAYING) {
        const bool livesLeft = std::any_of(
            m_players.begin(), m_players.end(), [](const auto &player) { return player && player->lives() > 0; });
        if (!livesLeft) {
            m_gameState = protocol::GAME_OVER;
            m_gameOverTimer = m_rules.gameOverDelay();
            std::cout << "Game over" << std::endl;
        }
    } else if (m_gameState == protocol::GAME_OVER) {
        m_gameOverTimer -= dt;
        if (m_gameOverTimer <= 0.f)
            restart();
    }
}

void Room::assignNetworkIds()
{
    auto &replicated = m_registry.getComponents<engine::ecs::Replicated>();
    for (std::size_t i : m_registry.view<engine::ecs::Replicated>()) {
        if (replicated.at(i).networkId() == 0)
            replicated.at(i).setNetworkId(nextNetworkId());
    }
}

void Room::handleDeaths()
{
    for (const engine::ecs::DeathEvent &death : m_deaths.take()) {
        const Entity victim = death.victim();
        const std::uint16_t networkId = networkIdOf(victim);
        if (networkId != 0)
            m_events.entityDestroyed(networkId);
        if (const auto slot = shipSlot(victim)) {
            m_ships[*slot].reset();
            m_events.playerKilled(*slot);
            m_players[*slot]->loseLife(m_rules.respawnDelay());
            std::cout << "Player " << *slot << " killed, " << static_cast<int>(m_players[*slot]->lives())
                      << " lives left" << std::endl;
        }
        if (!death.killer() || !m_registry.has<engine::ecs::ScoreValue>(victim))
            continue;
        const auto owner = ownerSlot(*death.killer());
        if (owner && m_players[*owner])
            m_players[*owner]->addScore(m_registry.get<engine::ecs::ScoreValue>(victim).points());
    }
}

void Room::spawnShip(std::size_t slot, bool invulnerable)
{
    const Entity ship = m_prefabs.spawn(m_rules.playerPrefab());
    const float y = m_rules.spawnY() + static_cast<float>(slot) * m_rules.spawnSpacing();
    if (m_registry.has<engine::ecs::Transform>(ship))
        m_registry.get<engine::ecs::Transform>(ship).setPosition(m_rules.spawnX(), y);
    else
        m_registry.emplace<engine::ecs::Transform>(ship, m_rules.spawnX(), y);
    if (m_registry.has<engine::ecs::Replicated>(ship))
        m_registry.get<engine::ecs::Replicated>(ship).setNetworkId(nextNetworkId());
    if (invulnerable)
        m_registry.emplace<engine::ecs::Invulnerable>(ship, m_rules.invulnerability());
    m_ships[slot] = ship;
    m_shipOwners.insert_or_assign(keyOf(ship), slot);
}

void Room::restart()
{
    clearWorld();
    m_shipOwners.clear();
    for (std::size_t slot = 0; slot < m_players.size(); ++slot) {
        if (!m_players[slot])
            continue;
        m_players[slot] = PlayerSlot(m_rules.startLives());
        if (m_ships[slot])
            m_shipOwners.insert_or_assign(keyOf(*m_ships[slot]), slot);
        else
            spawnShip(slot, false);
    }
    m_gameState = protocol::GAME_PLAYING;
    std::cout << "New round" << std::endl;
}

void Room::clearWorld()
{
    for (std::size_t i : m_registry.view<engine::ecs::Replicated>()) {
        const Entity entity = m_registry.entityAt(i);
        if (!shipSlot(entity))
            m_registry.kill(entity);
    }
}

bool Room::canSpawnReplicated() const
{
    return m_registry.view<engine::ecs::Replicated>().size() < protocol::MAX_ENTITIES;
}

std::optional<std::size_t> Room::shipSlot(const Entity &entity) const
{
    for (std::size_t slot = 0; slot < m_ships.size(); ++slot) {
        if (m_ships[slot] && *m_ships[slot] == entity)
            return slot;
    }
    return std::nullopt;
}

std::optional<std::size_t> Room::ownerSlot(const Entity &entity) const
{
    const auto found = m_shipOwners.find(keyOf(entity));
    if (found == m_shipOwners.end())
        return std::nullopt;
    return found->second;
}

std::uint16_t Room::networkIdOf(const Entity &entity) const
{
    if (!m_registry.has<engine::ecs::Replicated>(entity))
        return 0;
    return m_registry.get<engine::ecs::Replicated>(entity).networkId();
}

std::uint16_t Room::nextNetworkId()
{
    while (m_nextNetworkId == 0 || networkIdInUse(m_nextNetworkId))
        ++m_nextNetworkId;
    return m_nextNetworkId++;
}

bool Room::networkIdInUse(std::uint16_t networkId) const
{
    const auto &replicated = m_registry.getComponents<engine::ecs::Replicated>();
    for (std::size_t i : m_registry.view<engine::ecs::Replicated>()) {
        if (replicated.at(i).networkId() == networkId)
            return true;
    }
    return false;
}

Room::EntityKey Room::keyOf(const Entity &entity)
{
    return {entity.index(), entity.generation()};
}

} // namespace rtype::server
