#pragma once

#include <functional>

#include "Entity.hpp"
#include "ISystem.hpp"
#include "ServerWorld.hpp"

namespace rtype::server {

class CollisionSystem : public engine::ecs::ISystem
{
public:
    using Listener = std::function<void(const engine::ecs::Entity &, const engine::ecs::Entity &)>;

    CollisionSystem(ServerWorld &world, Listener onCollision);

    void update(float dt) override;

private:
    ServerWorld &m_world;
    Listener m_onCollision;
};

} // namespace rtype::server
