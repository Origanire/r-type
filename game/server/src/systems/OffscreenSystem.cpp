#include "OffscreenSystem.hpp"

#include "CommandBuffer.hpp"
#include "GameConstants.hpp"
#include "Hitbox.hpp"
#include "Position.hpp"
#include "Projectile.hpp"
#include "Registry.hpp"

namespace rtype::server {

// Leaving the world is not a destruction: no event is sent, the missile is only absent from the next snapshot.
void offscreenSystem(engine::ecs::SystemContext &context)
{
    auto &positions = context.registry.getComponents<Position>();
    auto &hitboxes = context.registry.getComponents<Hitbox>();
    auto &projectiles = context.registry.getComponents<Projectile>();
    for (std::size_t i = 0; i < positions.size() && i < hitboxes.size() && i < projectiles.size(); ++i) {
        if (!positions[i] || !hitboxes[i] || !projectiles[i])
            continue;
        const bool outside = positions[i]->x >= WORLD_WIDTH || positions[i]->x + hitboxes[i]->width <= 0.f ||
            positions[i]->y >= WORLD_HEIGHT || positions[i]->y + hitboxes[i]->height <= 0.f;
        if (outside)
            context.commands.kill(context.registry.entityFromIndex(i));
    }
}

} // namespace rtype::server
