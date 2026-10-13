#pragma once

#include <cstddef>
#include <cstdint>

#include "Entity.hpp"
#include "Registry.hpp"

namespace rtype::server {

engine::ecs::Entity createPlayerShip(engine::ecs::Registry &registry, std::size_t slot, std::uint16_t networkId);

} // namespace rtype::server
