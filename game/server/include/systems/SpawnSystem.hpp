#pragma once

#include <cstdint>
#include <functional>
#include <random>

#include "SystemContext.hpp"

namespace rtype::server {

// Spawns Bydos just beyond the right edge, at random heights, intervals and patterns.
class SpawnSystem
{
public:
    explicit SpawnSystem(std::function<std::uint16_t()> nextNetworkId);

    void operator()(engine::ecs::SystemContext &context);

private:
    float randomInterval();

    std::function<std::uint16_t()> m_nextNetworkId;
    std::mt19937 m_random;
    float m_timer;
};

} // namespace rtype::server
