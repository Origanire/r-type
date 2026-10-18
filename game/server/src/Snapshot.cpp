#include "Snapshot.hpp"

#include <cmath>
#include <cstdint>

namespace rtype::server {

namespace {

protocol::NetworkEntity toNetworkEntity(const ServerWorld &world, std::size_t i)
{
    protocol::NetworkEntity entity;
    entity.id = world.replicated[i].networkId;
    entity.kind = world.replicated[i].kind;
    entity.x = static_cast<std::int16_t>(std::lround(world.positions[i].x));
    entity.y = static_cast<std::int16_t>(std::lround(world.positions[i].y));
    return entity;
}

void collect(const ServerWorld &world, bool playerShips, std::vector<protocol::NetworkEntity> &entities)
{
    for (std::size_t i : world.entities.with(POSITION | REPLICATED)) {
        if (entities.size() >= protocol::MAX_ENTITIES)
            return;
        const bool isPlayerShip = world.replicated[i].kind == protocol::KIND_PLAYER_SHIP;
        if (isPlayerShip == playerShips)
            entities.push_back(toNetworkEntity(world, i));
    }
}

} // namespace

std::size_t countReplicated(const ServerWorld &world)
{
    return world.entities.with(REPLICATED).size();
}

std::vector<protocol::NetworkEntity> collectNetworkEntities(const ServerWorld &world)
{
    std::vector<protocol::NetworkEntity> entities;
    collect(world, true, entities);
    collect(world, false, entities);
    return entities;
}

} // namespace rtype::server
