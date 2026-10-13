#include "SnapshotSystem.hpp"

#include <cmath>
#include <cstdint>

#include "Position.hpp"
#include "Replicated.hpp"

namespace rtype::server {

std::vector<protocol::NetworkEntity> collectNetworkEntities(engine::ecs::Registry &registry)
{
    auto &positions = registry.getComponents<Position>();
    auto &replicated = registry.getComponents<Replicated>();
    std::vector<protocol::NetworkEntity> entities;
    for (std::size_t i = 0; i < positions.size() && i < replicated.size(); ++i) {
        if (!positions[i] || !replicated[i] || entities.size() >= protocol::MAX_ENTITIES)
            continue;
        protocol::NetworkEntity entity;
        entity.id = replicated[i]->networkId;
        entity.kind = replicated[i]->kind;
        entity.x = static_cast<std::int16_t>(std::lround(positions[i]->x));
        entity.y = static_cast<std::int16_t>(std::lround(positions[i]->y));
        entities.push_back(entity);
    }
    return entities;
}

} // namespace rtype::server
