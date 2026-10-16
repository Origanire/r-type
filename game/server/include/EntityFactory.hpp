#pragma once

#include <cstddef>
#include <cstdint>

#include "ServerWorld.hpp"

namespace rtype::server {

engine::ecs::Entity createPlayerShip(ServerWorld &world, std::size_t slot, std::uint16_t networkId);

} // namespace rtype::server
