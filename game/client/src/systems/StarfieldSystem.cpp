#include "StarfieldSystem.hpp"

#include "WorldSize.hpp"
#include "Zipper.hpp"

namespace rtype::client {

void starfieldSystem(engine::ecs::SystemContext &context, engine::ecs::SparseArray<Position> &positions,
    engine::ecs::SparseArray<Star> &stars)
{
    for (auto [position, star] : engine::ecs::Zipper(positions, stars)) {
        position.x -= star.speed * context.dt;
        if (position.x < -star.size)
            position.x += WORLD_WIDTH + star.size;
    }
}

} // namespace rtype::client
