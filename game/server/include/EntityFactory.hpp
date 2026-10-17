#pragma once

#include <cstddef>
#include <cstdint>

#include "Enemy.hpp"
#include "Entity.hpp"
#include "Position.hpp"
#include "Registry.hpp"

namespace rtype::server {

engine::ecs::Entity createPlayerShip(engine::ecs::Registry &registry, std::size_t slot, std::uint16_t networkId);
// Fills an entity already spawned, so that it can be called from a CommandBuffer spawn.
void createPlayerMissile(
    engine::ecs::Registry &registry, const engine::ecs::Entity &missile, Position origin, std::uint16_t networkId);
// goingUp only matters for a zigzag Bydo: it picks the first vertical direction.
void createBydo(engine::ecs::Registry &registry, const engine::ecs::Entity &bydo, Position origin, EnemyPattern pattern,
    bool goingUp, std::uint16_t networkId);

} // namespace rtype::server
