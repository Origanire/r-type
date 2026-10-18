#pragma once

#include "ISystem.hpp"
#include "ServerWorld.hpp"

namespace rtype::server {

class BoundarySystem : public engine::ecs::ISystem
{
public:
    explicit BoundarySystem(ServerWorld &world);

    void update(float dt) override;

private:
    ServerWorld &m_world;
};

} // namespace rtype::server
