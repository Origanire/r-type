#pragma once

#include "ClientWorld.hpp"
#include "ISystem.hpp"

namespace rtype::client {

class StarfieldSystem : public engine::ecs::ISystem
{
public:
    explicit StarfieldSystem(ClientWorld &world);

    void update(float dt) override;

private:
    ClientWorld &m_world;
};

} // namespace rtype::client
