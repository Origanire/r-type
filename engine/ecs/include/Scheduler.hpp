#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

#include "CommandBuffer.hpp"
#include "Registry.hpp"
#include "SystemContext.hpp"

namespace engine::ecs {

enum class Phase
{
    Input,
    Update,
    Physics,
    Render
};

class Scheduler
{
public:
    template <typename... Components, typename Function>
    void addSystem(Phase phase, Function &&function)
    {
        m_systems[static_cast<std::size_t>(phase)].emplace_back(
            [function = std::forward<Function>(function)](SystemContext &context) mutable {
                function(context, context.registry.getComponents<Components>()...);
            });
    }

    void run(Registry &registry, float dt);
    CommandBuffer &commands();

private:
    static constexpr std::size_t PHASE_COUNT = static_cast<std::size_t>(Phase::Render) + 1;

    std::array<std::vector<std::function<void(SystemContext &)>>, PHASE_COUNT> m_systems;
    CommandBuffer m_commands;
};

} // namespace engine::ecs
