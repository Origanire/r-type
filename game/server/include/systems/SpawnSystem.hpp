#pragma once

#include <cstdint>
#include <functional>
#include <random>

#include "ISystem.hpp"
#include "ServerWorld.hpp"

namespace rtype::server {

class SpawnSystem : public engine::ecs::ISystem
{
public:
    SpawnSystem(ServerWorld &world, std::function<std::uint16_t()> nextNetworkId, std::function<bool()> canSpawn);

    void update(float dt) override;

private:
    float randomInterval();

    ServerWorld &m_world;
    std::function<std::uint16_t()> m_nextNetworkId;
    std::function<bool()> m_canSpawn;
    std::mt19937 m_random;
    float m_timer;
};

} // namespace rtype::server
