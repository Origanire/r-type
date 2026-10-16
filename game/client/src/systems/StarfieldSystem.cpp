#include "StarfieldSystem.hpp"

#include "GameConstants.hpp"

namespace rtype::client {

StarfieldSystem::StarfieldSystem(ClientWorld &world) : m_world(world)
{
}

void StarfieldSystem::update(float dt)
{
    for (std::size_t i : m_world.entities.with(POSITION | STAR)) {
        Position &position = m_world.positions[i];
        const Star &star = m_world.stars[i];
        position.x -= star.speed * dt;
        if (position.x < -star.size)
            position.x += WORLD_WIDTH + star.size;
    }
}

} // namespace rtype::client
