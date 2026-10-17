#pragma once

#include <cstddef>
#include <vector>

#include "Registry.hpp"
#include "RtypeProtocol.hpp"

namespace rtype::server {

std::size_t countReplicated(engine::ecs::Registry &registry);
std::vector<protocol::NetworkEntity> collectNetworkEntities(engine::ecs::Registry &registry);

} // namespace rtype::server
