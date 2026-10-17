#pragma once

#include <cstddef>
#include <cstdint>

#include "Entity.hpp"
#include "Position.hpp"
#include "Registry.hpp"

namespace rtype::server {

engine::ecs::Entity createPlayerShip(engine::ecs::Registry &registry, std::size_t slot, std::uint16_t networkId);
// Fills an entity already spawned, so that it can be called from a CommandBuffer spawn.
void createPlayerMissile(
    engine::ecs::Registry &registry, const engine::ecs::Entity &missile, Position origin, std::uint16_t networkId);

} // namespace rtype::server
