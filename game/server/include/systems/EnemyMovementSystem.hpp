#pragma once

#include "ISystem.hpp"
#include "ServerWorld.hpp"

namespace rtype::server {

class EnemyMovementSystem : public engine::ecs::ISystem
{
public:
    explicit EnemyMovementSystem(ServerWorld &world);

    void update(float dt) override;

private:
    ServerWorld &m_world;
};

} // namespace rtype::server
