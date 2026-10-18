#pragma once

#include <cstdint>
#include <functional>

#include "ISystem.hpp"
#include "ServerWorld.hpp"

namespace rtype::server {

class ShootingSystem : public engine::ecs::ISystem
{
public:
    ShootingSystem(ServerWorld &world, std::function<std::uint16_t()> nextNetworkId);

    void update(float dt) override;

private:
    ServerWorld &m_world;
    std::function<std::uint16_t()> m_nextNetworkId;
};

} // namespace rtype::server
