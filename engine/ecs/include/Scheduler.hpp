#pragma once

#include <array>
#include <cstddef>
#include <functional>
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

using System = std::function<void(SystemContext &)>;

class Scheduler
{
public:
    void addSystem(Phase phase, System system);
    void run(Registry &registry, float dt);

private:
    static constexpr std::size_t PHASE_COUNT = static_cast<std::size_t>(Phase::Render) + 1;

    std::array<std::vector<System>, PHASE_COUNT> m_systems;
    CommandBuffer m_commands;
};

} // namespace engine::ecs
