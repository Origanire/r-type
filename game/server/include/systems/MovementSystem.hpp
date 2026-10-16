#pragma once

#include "ISystem.hpp"
#include "ServerWorld.hpp"

namespace rtype::server {

class MovementSystem : public engine::ecs::ISystem
{
public:
    explicit MovementSystem(ServerWorld &world);

    void update(float dt) override;

private:
    ServerWorld &m_world;
};

} // namespace rtype::server
