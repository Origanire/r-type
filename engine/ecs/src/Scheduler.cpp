#include "Scheduler.hpp"

#include <utility>

#include "EcsError.hpp"

namespace engine::ecs {

void Scheduler::addSystem(Phase phase, System system)
{
    if (!system)
        throw EcsError("Scheduler: empty system");
    m_systems[static_cast<std::size_t>(phase)].push_back(std::move(system));
}

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

} // namespace engine::ecs
