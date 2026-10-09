#include "CommandBuffer.hpp"

#include <utility>

#include "Registry.hpp"

namespace engine::ecs {

void CommandBuffer::spawn(SpawnSetup setup)
{
    m_commands.emplace_back([setup = std::move(setup)](Registry &registry) {
        Entity entity = registry.spawnEntity();
        if (setup)
            setup(registry, entity);
    });
}

void CommandBuffer::kill(const Entity &entity)
{
    m_commands.emplace_back([entity](Registry &registry) { registry.killEntity(entity); });
}

void CommandBuffer::apply(Registry &registry)
{
    std::vector<std::function<void(Registry &)>> commands = std::move(m_commands);
    m_commands.clear();
    for (auto &command : commands)
        command(registry);
}

std::size_t CommandBuffer::size() const
{
    return m_commands.size();
}

} // namespace engine::ecs
