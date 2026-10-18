#include "MovementSystem.hpp"

namespace rtype::server {

MovementSystem::MovementSystem(ServerWorld &world) : m_world(world)
{
}

void MovementSystem::update(float dt)
{
    for (std::size_t i : m_world.entities.with(POSITION | VELOCITY)) {
        m_world.positions[i].x += m_world.velocities[i].x * dt;
        m_world.positions[i].y += m_world.velocities[i].y * dt;
    }
}

} // namespace rtype::server
