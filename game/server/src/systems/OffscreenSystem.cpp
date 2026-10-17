#include "OffscreenSystem.hpp"

#include "CommandBuffer.hpp"
#include "Enemy.hpp"
#include "GameConstants.hpp"
#include "Hitbox.hpp"
#include "Position.hpp"
#include "Projectile.hpp"
#include "Registry.hpp"

namespace rtype::server {

// Leaving the world is not a destruction: no event is sent, the entity is only absent from the next snapshot.
// Bydos spawn beyond the right edge, so for them only the left edge counts.
void offscreenSystem(engine::ecs::SystemContext &context)
{
    auto &positions = context.registry.getComponents<Position>();
    auto &hitboxes = context.registry.getComponents<Hitbox>();
    auto &projectiles = context.registry.getComponents<Projectile>();
    auto &enemies = context.registry.getComponents<Enemy>();
    for (std::size_t i = 0; i < positions.size() && i < hitboxes.size(); ++i) {
        if (!positions[i] || !hitboxes[i])
            continue;
        const bool projectile = i < projectiles.size() && projectiles[i];
        const bool enemy = i < enemies.size() && enemies[i];
        const bool pastLeft = positions[i]->x + hitboxes[i]->width <= 0.f;
        const bool outside = pastLeft || positions[i]->x >= WORLD_WIDTH || positions[i]->y >= WORLD_HEIGHT ||
            positions[i]->y + hitboxes[i]->height <= 0.f;
        if ((projectile && outside) || (enemy && pastLeft))
            context.commands.kill(context.registry.entityFromIndex(i));
    }
}

} // namespace rtype::server
