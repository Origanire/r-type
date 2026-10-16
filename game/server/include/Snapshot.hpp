#pragma once

#include <vector>

#include "RtypeProtocol.hpp"
#include "ServerWorld.hpp"

namespace rtype::server {

std::vector<protocol::NetworkEntity> collectNetworkEntities(const ServerWorld &world);

} // namespace rtype::server
