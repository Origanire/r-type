#pragma once

#include "ISystem.hpp"
#include "ServerWorld.hpp"

namespace rtype::server {

class OffscreenSystem : public engine::ecs::ISystem
{
public:
    explicit OffscreenSystem(ServerWorld &world);

    void update(float dt) override;

private:
    ServerWorld &m_world;
};

} // namespace rtype::server
