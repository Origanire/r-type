#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "CleanupSystem.hpp"
#include "CollisionEvent.hpp"
#include "ComponentFactory.hpp"
#include "DataNode.hpp"
#include "DeathEvent.hpp"
#include "Entity.hpp"
#include "EventQueue.hpp"
#include "GameEvents.hpp"
#include "GameRules.hpp"
#include "LayerTable.hpp"
#include "PlayerSlot.hpp"
#include "PrefabLibrary.hpp"
#include "Registry.hpp"
#include "RtypeProtocol.hpp"
#include "SystemManager.hpp"

namespace rtype::server {

class Room
{
public:
    explicit Room(const std::filesystem::path &dataDirectory);

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
    using EntityKey = std::pair<std::size_t, std::uint32_t>;

    void registerSystems();
    void updateRespawns(float dt);
    void updateGameState(float dt);
    void assignNetworkIds();
    void handleDeaths();
    void spawnShip(std::size_t slot, bool invulnerable);
    void restart();
    void clearWorld();
    bool canSpawnReplicated() const;
    std::optional<std::size_t> shipSlot(const engine::ecs::Entity &entity) const;
    std::optional<std::size_t> ownerSlot(const engine::ecs::Entity &entity) const;
    std::uint16_t networkIdOf(const engine::ecs::Entity &entity) const;
    std::uint16_t nextNetworkId();
    bool networkIdInUse(std::uint16_t networkId) const;
    static EntityKey keyOf(const engine::ecs::Entity &entity);

    engine::scene::DataNode m_config;
    GameRules m_rules;
    engine::ecs::Registry m_registry;
    engine::scene::LayerTable m_layers;
    engine::scene::ComponentFactory m_factory;
    engine::scene::PrefabLibrary m_prefabs;
    engine::EventQueue<engine::ecs::CollisionEvent> m_collisions;
    engine::EventQueue<engine::ecs::DeathEvent> m_deaths;
    engine::ecs::SystemManager m_systems;
    engine::ecs::CleanupSystem m_cleanup;
    GameEvents m_events;
    std::array<std::optional<PlayerSlot>, protocol::MAX_PLAYERS> m_players;
    std::array<std::optional<engine::ecs::Entity>, protocol::MAX_PLAYERS> m_ships;
    std::map<EntityKey, std::size_t> m_shipOwners;
    std::uint8_t m_gameState = protocol::GAME_WAITING;
    float m_gameOverTimer = 0.f;
    std::uint16_t m_nextNetworkId = 1;
    std::uint32_t m_tick = 0;
};

} // namespace rtype::server
