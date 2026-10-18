#include "EnemyMovementSystem.hpp"

#include <cmath>

#include "GameConstants.hpp"

namespace rtype::server {

EnemyMovementSystem::EnemyMovementSystem(ServerWorld &world) : m_world(world)
{
}

void EnemyMovementSystem::update(float dt)
{
    for (std::size_t i : m_world.entities.with(ENEMY | POSITION | VELOCITY | HITBOX)) {
        Enemy &enemy = m_world.enemies[i];
        const Position &position = m_world.positions[i];
        Velocity &velocity = m_world.velocities[i];
        enemy.age += dt;
        velocity.x = -BYDO_SPEED;
        switch (enemy.pattern) {
            case EnemyPattern::Straight:
                velocity.y = 0.f;
                break;
            case EnemyPattern::Sine: {
                const float target = enemy.baseY + BYDO_SINE_AMPLITUDE * std::sin(BYDO_SINE_FREQUENCY * enemy.age);
                velocity.y = (target - position.y) / dt;
                break;
            }
            case EnemyPattern::Zigzag:
                if (position.y <= 0.f)
                    velocity.y = BYDO_ZIGZAG_SPEED;
                else if (position.y + m_world.hitboxes[i].height >= WORLD_HEIGHT)
                    velocity.y = -BYDO_ZIGZAG_SPEED;
                break;
        }
    }
}

} // namespace rtype::server
