#include "Room.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>

#include "BoundarySystem.hpp"
#include "CollisionSystem.hpp"
#include "EnemyMovementSystem.hpp"
#include "EntityFactory.hpp"
#include "GameConstants.hpp"
#include "GameError.hpp"
#include "InputSystem.hpp"
#include "InvulnerabilitySystem.hpp"
#include "MovementSystem.hpp"
#include "OffscreenSystem.hpp"
#include "ShootingSystem.hpp"
#include "Snapshot.hpp"
#include "SpawnSystem.hpp"

namespace rtype::server {

Room::Room()
{
    m_systems.add(std::make_unique<InputSystem>(m_world));
    m_systems.add(std::make_unique<ShootingSystem>(m_world, [this] { return nextNetworkId(); }));
    m_systems.add(std::make_unique<SpawnSystem>(
        m_world, [this] { return nextNetworkId(); }, [this] { return m_gameState == protocol::GAME_PLAYING; }));
    m_systems.add(std::make_unique<EnemyMovementSystem>(m_world));
    m_systems.add(std::make_unique<InvulnerabilitySystem>(m_world));
    m_systems.add(std::make_unique<MovementSystem>(m_world));
    m_systems.add(std::make_unique<BoundarySystem>(m_world));
    m_systems.add(std::make_unique<CollisionSystem>(m_world,
        [this](const engine::ecs::Entity &first, const engine::ecs::Entity &second) { onCollision(first, second); }));
    m_systems.add(std::make_unique<OffscreenSystem>(m_world));
}

void Room::spawnPlayer(std::size_t slot)
{
    if (slot >= m_players.size() || m_players[slot])
        throw GameError("Room: invalid or occupied player slot");
    m_players[slot] = Player{PLAYER_START_LIVES};
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
        m_world.entities.destroy(*m_ships[slot]);
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
    m_world.inputs[m_ships[slot]->index()].buttons = buttons;
}

void Room::update(float dt)
{
    updateRespawns(dt);
    m_systems.update(dt);
    updateGameState(dt);
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
        states[slot] = {state, m_players[slot]->lives, m_players[slot]->score, shipId};
    }
    return states;
}

std::vector<protocol::NetworkEntity> Room::networkEntities() const
{
    return collectNetworkEntities(m_world);
}

std::vector<protocol::Packet> Room::takeEvents()
{
    return m_events.take();
}

void Room::updateRespawns(float dt)
{
    if (m_gameState != protocol::GAME_PLAYING)
        return;
    for (std::size_t slot = 0; slot < m_players.size(); ++slot) {
        if (!m_players[slot] || m_ships[slot] || m_players[slot]->lives == 0)
            continue;
        m_players[slot]->respawnTimer -= dt;
        if (m_players[slot]->respawnTimer <= 0.f)
            spawnShip(slot, true);
    }
}

void Room::updateGameState(float dt)
{
    if (m_gameState == protocol::GAME_PLAYING) {
        const bool livesLeft = std::any_of(
            m_players.begin(), m_players.end(), [](const auto &player) { return player && player->lives > 0; });
        if (!livesLeft) {
            m_gameState = protocol::GAME_OVER;
            m_gameOverTimer = GAME_OVER_DELAY;
            std::cout << "Game over" << std::endl;
        }
    } else if (m_gameState == protocol::GAME_OVER) {
        m_gameOverTimer -= dt;
        if (m_gameOverTimer <= 0.f)
            restart();
    }
}

void Room::spawnShip(std::size_t slot, bool invulnerable)
{
    m_ships[slot] = createPlayerShip(m_world, slot, nextNetworkId());
    if (!invulnerable)
        return;
    m_world.invulnerables[m_ships[slot]->index()] = Invulnerable{PLAYER_INVULNERABILITY};
    m_world.entities.addComponents(*m_ships[slot], INVULNERABLE);
}

void Room::restart()
{
    clearWorld();
    for (std::size_t slot = 0; slot < m_players.size(); ++slot) {
        if (!m_players[slot])
            continue;
        m_players[slot] = Player{PLAYER_START_LIVES};
        if (!m_ships[slot])
            spawnShip(slot, false);
    }
    m_gameState = protocol::GAME_PLAYING;
    std::cout << "New round" << std::endl;
}

void Room::clearWorld()
{
    destroyAll(ENEMY);
    destroyAll(PROJECTILE);
}

void Room::onCollision(const engine::ecs::Entity &first, const engine::ecs::Entity &second)
{
    if (!m_world.entities.isAlive(first) || !m_world.entities.isAlive(second))
        return;
    for (const auto &[a, b] : {std::pair{first, second}, std::pair{second, first}}) {
        if (!m_world.entities.hasComponents(b, ENEMY))
            continue;
        if (m_world.entities.hasComponents(a, PROJECTILE))
            hitEnemy(a, b);
        else if (m_world.entities.hasComponents(a, PLAYER_SHIP))
            hitShip(a, b);
        return;
    }
}

void Room::hitEnemy(const engine::ecs::Entity &missile, const engine::ecs::Entity &enemy)
{
    const std::size_t owner = m_world.projectiles[missile.index()].ownerSlot;
    m_world.entities.destroy(missile);
    if (m_world.entities.hasComponents(enemy, HEALTH)) {
        Health &health = m_world.healths[enemy.index()];
        health.points -= MISSILE_DAMAGE;
        if (health.points > 0)
            return;
    }
    destroy(enemy);
    if (owner < m_players.size() && m_players[owner])
        m_players[owner]->score += BYDO_SCORE;
}

void Room::hitShip(const engine::ecs::Entity &ship, const engine::ecs::Entity &enemy)
{
    if (m_world.entities.hasComponents(ship, INVULNERABLE))
        return;
    const std::size_t slot = m_world.playerShips[ship.index()].slot;
    destroy(enemy);
    destroy(ship);
    m_ships[slot].reset();
    m_events.playerKilled(slot);
    Player &player = *m_players[slot];
    if (player.lives > 0)
        --player.lives;
    player.respawnTimer = PLAYER_RESPAWN_DELAY;
    std::cout << "Player " << slot << " killed, " << static_cast<int>(player.lives) << " lives left" << std::endl;
}

void Room::destroy(const engine::ecs::Entity &entity)
{
    m_events.entityDestroyed(networkIdOf(entity));
    m_world.entities.destroy(entity);
}

void Room::destroyAll(engine::ecs::ComponentMask components)
{
    for (std::size_t i : m_world.entities.with(components))
        m_world.entities.destroy(m_world.entities.entityAt(i));
}

std::uint16_t Room::networkIdOf(const engine::ecs::Entity &entity) const
{
    if (!m_world.entities.hasComponents(entity, REPLICATED))
        return 0;
    return m_world.replicated[entity.index()].networkId;
}

std::uint16_t Room::nextNetworkId()
{
    while (m_nextNetworkId == 0 || networkIdInUse(m_nextNetworkId))
        ++m_nextNetworkId;
    return m_nextNetworkId++;
}

bool Room::networkIdInUse(std::uint16_t networkId) const
{
    for (std::size_t i : m_world.entities.with(REPLICATED)) {
        if (m_world.replicated[i].networkId == networkId)
            return true;
    }
    return false;
}

} // namespace rtype::server
