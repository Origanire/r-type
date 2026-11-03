#pragma once

#include <memory>

namespace engine::scene {

template <class System>
std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildSimple(SystemContext &context, const DataNode &)
{
    return std::make_unique<System>(context.registry());
}

template <class System>
std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildSpawning(SystemContext &context, const DataNode &)
{
    return std::make_unique<System>(context.registry(), context.factory(), context.canSpawn());
}

template <class System>
std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildDeathListener(SystemContext &context, const DataNode &)
{
    return std::make_unique<System>(context.registry(), context.deaths());
}

} // namespace engine::scene
