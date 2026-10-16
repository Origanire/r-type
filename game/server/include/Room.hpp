#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

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
    std::uint16_t playerNetworkId(std::size_t slot) const;
    std::vector<protocol::NetworkEntity> networkEntities() const;

private:
    std::uint16_t nextNetworkId();
    bool networkIdInUse(std::uint16_t networkId) const;

    ServerWorld m_world;
    engine::ecs::SystemManager m_systems;
    std::array<std::optional<engine::ecs::Entity>, protocol::MAX_PLAYERS> m_ships;
    std::uint16_t m_nextNetworkId = 1;
    std::uint32_t m_tick = 0;
};

} // namespace rtype::server
