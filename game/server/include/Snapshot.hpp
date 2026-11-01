#pragma once

#include <vector>

#include "Registry.hpp"
#include "RtypeProtocol.hpp"

namespace rtype::server {

std::vector<protocol::NetworkEntity> collectNetworkEntities(const engine::ecs::Registry &registry);

} // namespace rtype::server
