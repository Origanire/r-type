#include "BoundarySystem.hpp"

#include <algorithm>

#include "GameConstants.hpp"
#include "Hitbox.hpp"
#include "PlayerInput.hpp"
#include "Position.hpp"
#include "Registry.hpp"

namespace rtype::server {

void boundarySystem(engine::ecs::SystemContext &context)
{
    auto &positions = context.registry.getComponents<Position>();
    auto &hitboxes = context.registry.getComponents<Hitbox>();
    auto &inputs = context.registry.getComponents<PlayerInput>();
    for (std::size_t i = 0; i < positions.size() && i < hitboxes.size() && i < inputs.size(); ++i) {
        if (!positions[i] || !hitboxes[i] || !inputs[i])
            continue;
        positions[i]->x = std::clamp(positions[i]->x, 0.f, WORLD_WIDTH - hitboxes[i]->width);
        positions[i]->y = std::clamp(positions[i]->y, 0.f, WORLD_HEIGHT - hitboxes[i]->height);
    }
}

} // namespace rtype::server
