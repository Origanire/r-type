#pragma once

#include <cstddef>
#include <cstdint>

#include "Enemy.hpp"
#include "Position.hpp"
#include "ServerWorld.hpp"

namespace rtype::server {

engine::ecs::Entity createPlayerShip(ServerWorld &world, std::size_t slot, std::uint16_t networkId);
engine::ecs::Entity createPlayerMissile(
    ServerWorld &world, Position origin, std::size_t ownerSlot, std::uint16_t networkId);
engine::ecs::Entity createBydo(
    ServerWorld &world, Position origin, EnemyPattern pattern, bool goingUp, std::uint16_t networkId);

} // namespace rtype::server
