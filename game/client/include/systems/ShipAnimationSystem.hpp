#pragma once

#include "ClientWorld.hpp"
#include "ISystem.hpp"

namespace rtype::client {

class ShipAnimationSystem : public engine::ecs::ISystem
{
public:
    explicit ShipAnimationSystem(ClientWorld &world);

    void update(float dt) override;

private:
    ClientWorld &m_world;
};

} // namespace rtype::client
