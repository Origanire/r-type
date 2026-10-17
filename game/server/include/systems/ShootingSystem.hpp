#pragma once

#include <cstdint>
#include <functional>

#include "SystemContext.hpp"

namespace rtype::server {

class ShootingSystem
{
public:
    explicit ShootingSystem(std::function<std::uint16_t()> nextNetworkId);

    void operator()(engine::ecs::SystemContext &context) const;

private:
    std::function<std::uint16_t()> m_nextNetworkId;
};

} // namespace rtype::server
