#pragma once

#include <memory>

#include "DataNode.hpp"
#include "ISystem.hpp"
#include "SystemContext.hpp"
#include "SystemFactory.hpp"

namespace engine::scene {

class EngineSystemBuilders
{
public:
    static void registerAll(SystemFactory &factory);

private:
    static void registerSimulation(SystemFactory &factory);
    static void registerRules(SystemFactory &factory);
    static void registerPresentation(SystemFactory &factory);

    template <class System>
    static std::unique_ptr<ecs::ISystem> buildSimple(SystemContext &context, const DataNode &entry);
    template <class System>
    static std::unique_ptr<ecs::ISystem> buildSpawning(SystemContext &context, const DataNode &entry);
    template <class System>
    static std::unique_ptr<ecs::ISystem> buildDeathListener(SystemContext &context, const DataNode &entry);

    static std::unique_ptr<ecs::ISystem> buildCollision(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildDamage(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildRespawn(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildDeathEffect(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildPlayerLabel(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildAudio(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildRender(SystemContext &context, const DataNode &entry);
};

} // namespace engine::scene

#include "EngineSystemBuilders.tpp"
