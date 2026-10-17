#include "EnemyMovementSystem.hpp"

#include <cmath>

#include "Enemy.hpp"
#include "GameConstants.hpp"
#include "Hitbox.hpp"
#include "Position.hpp"
#include "Registry.hpp"
#include "Velocity.hpp"

namespace rtype::server {

// Only sets velocities: the movement system applies them in the physics phase.
void enemyMovementSystem(engine::ecs::SystemContext &context)
{
    auto &enemies = context.registry.getComponents<Enemy>();
    auto &positions = context.registry.getComponents<Position>();
    auto &velocities = context.registry.getComponents<Velocity>();
    auto &hitboxes = context.registry.getComponents<Hitbox>();
    for (std::size_t i = 0; i < enemies.size() && i < positions.size() && i < velocities.size() && i < hitboxes.size();
        ++i) {
        if (!enemies[i] || !positions[i] || !velocities[i] || !hitboxes[i])
            continue;
        enemies[i]->age += context.dt;
        velocities[i]->x = -BYDO_SPEED;
        switch (enemies[i]->pattern) {
            case EnemyPattern::Straight:
                velocities[i]->y = 0.f;
                break;
            case EnemyPattern::Sine: {
                // Aims at the wave height of the next step, so that rounding never drifts away from it.
                const float target = enemies[i]->baseY +
                    BYDO_SINE_AMPLITUDE * std::sin(BYDO_SINE_FREQUENCY * enemies[i]->age);
                velocities[i]->y = (target - positions[i]->y) / context.dt;
                break;
            }
            case EnemyPattern::Zigzag:
                if (positions[i]->y <= 0.f)
                    velocities[i]->y = BYDO_ZIGZAG_SPEED;
                else if (positions[i]->y + hitboxes[i]->height >= WORLD_HEIGHT)
                    velocities[i]->y = -BYDO_ZIGZAG_SPEED;
                break;
        }
    }
}

} // namespace rtype::server
