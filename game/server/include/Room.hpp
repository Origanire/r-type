#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "Entity.hpp"
#include "GameEvents.hpp"
#include "RtypeProtocol.hpp"
#include "ServerWorld.hpp"
#include "SystemManager.hpp"

namespace rtype::server {

class Room
{
public:
    Room();

    void spawnPlayer(std::size_t slot);
    void removePlayer(std::size_t slot);
    void setButtons(std::size_t slot, std::uint8_t buttons);
    void update(float dt);

    std::uint32_t tick() const;
    std::uint8_t gameState() const;
    std::array<protocol::PlayerState, protocol::MAX_PLAYERS> playerStates() const;
    std::vector<protocol::NetworkEntity> networkEntities() const;
    std::vector<protocol::Packet> takeEvents();

private:
    struct Player
    {
        std::uint8_t lives = 0;
        std::uint32_t score = 0;
        float respawnTimer = 0.f;
    };

    void updateRespawns(float dt);
    void updateGameState(float dt);
    void spawnShip(std::size_t slot, bool invulnerable);
    void restart();
    void clearWorld();
    void onCollision(const engine::ecs::Entity &first, const engine::ecs::Entity &second);
    void hitEnemy(const engine::ecs::Entity &missile, const engine::ecs::Entity &enemy);
    void hitShip(const engine::ecs::Entity &ship, const engine::ecs::Entity &enemy);
    void destroy(const engine::ecs::Entity &entity);
    void destroyAll(engine::ecs::ComponentMask components);
    std::uint16_t networkIdOf(const engine::ecs::Entity &entity) const;
    std::uint16_t nextNetworkId();
    bool networkIdInUse(std::uint16_t networkId) const;

    ServerWorld m_world;
    engine::ecs::SystemManager m_systems;
    GameEvents m_events;
    std::array<std::optional<Player>, protocol::MAX_PLAYERS> m_players;
    std::array<std::optional<engine::ecs::Entity>, protocol::MAX_PLAYERS> m_ships;
    std::uint8_t m_gameState = protocol::GAME_WAITING;
    float m_gameOverTimer = 0.f;
    std::uint16_t m_nextNetworkId = 1;
    std::uint32_t m_tick = 0;
};

} // namespace rtype::server
