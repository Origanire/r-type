#pragma once

#include "ISystem.hpp"
#include "ServerWorld.hpp"

namespace rtype::server {

class InputSystem : public engine::ecs::ISystem
{
public:
    explicit InputSystem(ServerWorld &world);

    void update(float dt) override;

private:
    ServerWorld &m_world;
};

} // namespace rtype::server
