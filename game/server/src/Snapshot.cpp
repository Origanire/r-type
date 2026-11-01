#include "Snapshot.hpp"

#include <cmath>
#include <cstdint>

#include "Replicated.hpp"
#include "Transform.hpp"

namespace rtype::server {

namespace {

void collect(const engine::ecs::Registry &registry, bool playerShips, std::vector<protocol::NetworkEntity> &entities)
{
    const auto &transforms = registry.getComponents<engine::ecs::Transform>();
    const auto &replicated = registry.getComponents<engine::ecs::Replicated>();
    for (std::size_t i : registry.view<engine::ecs::Transform, engine::ecs::Replicated>()) {
        if (entities.size() >= protocol::MAX_ENTITIES)
            return;
        const engine::ecs::Replicated &identity = replicated.at(i);
        const bool isPlayerShip = identity.kind() == protocol::KIND_PLAYER_SHIP;
        if (identity.networkId() == 0 || isPlayerShip != playerShips)
            continue;
        protocol::NetworkEntity entity;
        entity.id = identity.networkId();
        entity.kind = identity.kind();
        entity.x = static_cast<std::int16_t>(std::lround(transforms.at(i).x()));
        entity.y = static_cast<std::int16_t>(std::lround(transforms.at(i).y()));
        entities.push_back(entity);
    }
}

} // namespace

std::vector<protocol::NetworkEntity> collectNetworkEntities(const engine::ecs::Registry &registry)
{
    std::vector<protocol::NetworkEntity> entities;
    collect(registry, true, entities);
    collect(registry, false, entities);
    return entities;
}

} // namespace rtype::server
