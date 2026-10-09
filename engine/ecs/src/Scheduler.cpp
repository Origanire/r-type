#include "Scheduler.hpp"

#include "EcsError.hpp"

namespace engine::ecs {

void Scheduler::run(Registry &registry, float dt)
{
    if (dt < 0.f)
        throw EcsError("Scheduler: negative delta time");
    SystemContext context{registry, m_commands, dt};
    for (auto &phase : m_systems) {
        for (auto &system : phase)
            system(context);
    }
    m_commands.apply(registry);
}

CommandBuffer &Scheduler::commands()
{
    return m_commands;
}

} // namespace engine::ecs
