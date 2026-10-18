#include "Room.hpp"

#include <algorithm>
#include <iostream>

#include "BoundarySystem.hpp"
#include "Enemy.hpp"
#include "EnemyMovementSystem.hpp"
#include "EntityFactory.hpp"
#include "GameConstants.hpp"
#include "GameError.hpp"
#include "Health.hpp"
#include "Hitbox.hpp"
#include "InputSystem.hpp"
#include "Invulnerable.hpp"
#include "InvulnerabilitySystem.hpp"
#include "MovementSystem.hpp"
#include "OffscreenSystem.hpp"
#include "PlayerInput.hpp"
#include "PlayerShip.hpp"
#include "Position.hpp"
#include "Projectile.hpp"
#include "Replicated.hpp"
#include "Shooter.hpp"
#include "ShootingSystem.hpp"
#include "SnapshotSystem.hpp"
#include "SpawnSystem.hpp"
#include "Velocity.hpp"

namespace rtype::server {

using engine::ecs::Phase;

Room::Room()
{
    m_registry.registerComponent<Position>();
    m_registry.registerComponent<Velocity>();
    m_registry.registerComponent<Hitbox>();
    m_registry.registerComponent<PlayerInput>();
    m_registry.registerComponent<Replicated>();
    m_registry.registerComponent<Shooter>();
    m_registry.registerComponent<Projectile>();
    m_registry.registerComponent<Enemy>();
    m_registry.registerComponent<Health>();
    m_registry.registerComponent<PlayerShip>();
    m_registry.registerComponent<Invulnerable>();
    m_scheduler.addSystem(Phase::Input, inputSystem);
    m_scheduler.addSystem(Phase::Update, ShootingSystem([this] { return nextNetworkId(); }));
    m_scheduler.addSystem(Phase::Update,
        SpawnSystem([this] { return nextNetworkId(); }, [this] { return m_gameState == protocol::GAME_PLAYING; }));
    m_scheduler.addSystem(Phase::Update, enemyMovementSystem);
    m_scheduler.addSystem(Phase::Update, invulnerabilitySystem);
    m_scheduler.addSystem(Phase::Physics, engine::physics::movementSystem);
    m_scheduler.addSystem(Phase::Physics, boundarySystem);
    m_scheduler.addSystem(Phase::Physics,
        engine::physics::CollisionSystem([this](const engine::physics::Collision &collision) { onCollision(collision); }));
    m_scheduler.addSystem(Phase::Physics, offscreenSystem);
}

void Room::spawnPlayer(std::size_t slot)
{
    if (slot >= m_players.size() || m_players[slot])
        throw GameError("Room: invalid or occupied player slot");
    m_players[slot] = Player{PLAYER_START_LIVES};
    if (m_gameState == protocol::GAME_WAITING)
        m_gameState = protocol::GAME_PLAYING;
    // During a game over, the newcomer waits for the next round, which spawns every ship.
    if (m_gameState == protocol::GAME_PLAYING)
        spawnShip(slot, false);
}

void Room::removePlayer(std::size_t slot)
{
    if (slot >= m_players.size() || !m_players[slot])
        return;
    if (m_ships[slot])
        m_registry.killEntity(*m_ships[slot]);
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
    auto &input = m_registry.getComponents<PlayerInput>()[*m_ships[slot]];
    if (input)
        input->buttons = buttons;
}

void Room::update(float dt)
{
    updateRespawns(dt);
    m_scheduler.run(m_registry, dt);
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

// A player without a ship is dead: waiting for a respawn, out of lives, or waiting for the next round.
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

std::vector<protocol::NetworkEntity> Room::networkEntities()
{
    return collectNetworkEntities(m_registry);
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
    m_ships[slot] = createPlayerShip(m_registry, slot, nextNetworkId());
    if (invulnerable)
        m_registry.addComponent(*m_ships[slot], Invulnerable{PLAYER_INVULNERABILITY});
}

// A new round: an empty world, full lives and a zero score for everyone present.
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
    auto &enemies = m_registry.getComponents<Enemy>();
    auto &projectiles = m_registry.getComponents<Projectile>();
    const std::size_t count = std::max(enemies.size(), projectiles.size());
    for (std::size_t i = 0; i < count; ++i) {
        if ((i < enemies.size() && enemies[i]) || (i < projectiles.size() && projectiles[i]))
            m_registry.killEntity(m_registry.entityFromIndex(i));
    }
}

// Collisions are reported after detection, so an earlier one of the same tick may have killed either entity:
// checking that both are alive resolves each impact only once.
void Room::onCollision(const engine::physics::Collision &collision)
{
    if (!m_registry.isAlive(collision.first) || !m_registry.isAlive(collision.second))
        return;
    const auto &projectiles = m_registry.getComponents<Projectile>();
    const auto &enemies = m_registry.getComponents<Enemy>();
    const auto &ships = m_registry.getComponents<PlayerShip>();
    for (const auto &[a, b] : {std::pair{collision.first, collision.second}, std::pair{collision.second, collision.first}}) {
        if (!enemies[b])
            continue;
        if (projectiles[a])
            hitEnemy(a, b);
        else if (ships[a])
            hitShip(a, b);
        return;
    }
}

void Room::hitEnemy(const engine::ecs::Entity &missile, const engine::ecs::Entity &enemy)
{
    const std::size_t owner = m_registry.getComponents<Projectile>()[missile]->ownerSlot;
    // A spent missile only disappears from the next snapshot, like one leaving the screen.
    m_registry.killEntity(missile);
    auto &health = m_registry.getComponents<Health>()[enemy];
    if (health) {
        health->points -= MISSILE_DAMAGE;
        if (health->points > 0)
            return;
    }
    destroy(enemy);
    if (owner < m_players.size() && m_players[owner])
        m_players[owner]->score += BYDO_SCORE;
}

void Room::hitShip(const engine::ecs::Entity &ship, const engine::ecs::Entity &enemy)
{
    if (m_registry.getComponents<Invulnerable>()[ship])
        return;
    const std::size_t slot = m_registry.getComponents<PlayerShip>()[ship]->slot;
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

// Unlike leaving the screen, a destruction is announced so that clients can play an explosion.
void Room::destroy(const engine::ecs::Entity &entity)
{
    m_events.entityDestroyed(networkIdOf(entity));
    m_registry.killEntity(entity);
}

std::uint16_t Room::networkIdOf(const engine::ecs::Entity &entity) const
{
    const auto &replicated = m_registry.getComponents<Replicated>()[entity];
    return replicated ? replicated->networkId : 0;
}

std::uint16_t Room::nextNetworkId()
{
    while (m_nextNetworkId == 0 || networkIdInUse(m_nextNetworkId))
        ++m_nextNetworkId;
    return m_nextNetworkId++;
}

bool Room::networkIdInUse(std::uint16_t networkId) const
{
    const auto &replicated = m_registry.getComponents<Replicated>();
    for (std::size_t i = 0; i < replicated.size(); ++i) {
        if (replicated[i] && replicated[i]->networkId == networkId)
            return true;
    }
    return false;
}

} // namespace rtype::server
