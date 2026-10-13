#include "StarfieldSystem.hpp"

#include "GameConstants.hpp"
#include "Position.hpp"
#include "Registry.hpp"
#include "Star.hpp"

namespace rtype::client {

void starfieldSystem(engine::ecs::SystemContext &context)
{
    auto &positions = context.registry.getComponents<Position>();
    auto &stars = context.registry.getComponents<Star>();
    for (std::size_t i = 0; i < positions.size() && i < stars.size(); ++i) {
        if (!positions[i] || !stars[i])
            continue;
        positions[i]->x -= stars[i]->speed * context.dt;
        if (positions[i]->x < -stars[i]->size)
            positions[i]->x += WORLD_WIDTH + stars[i]->size;
    }
}

} // namespace rtype::client
