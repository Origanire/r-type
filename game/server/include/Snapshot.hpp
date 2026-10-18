#pragma once

#include <cstddef>
#include <vector>

#include "RtypeProtocol.hpp"
#include "ServerWorld.hpp"

namespace rtype::server {

std::size_t countReplicated(const ServerWorld &world);
std::vector<protocol::NetworkEntity> collectNetworkEntities(const ServerWorld &world);

} // namespace rtype::server
