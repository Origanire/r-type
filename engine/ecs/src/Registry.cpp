#include "Registry.hpp"

namespace engine::ecs {

Entity Registry::spawnEntity()
{
    if (!m_freeIndexes.empty()) {
        std::size_t index = m_freeIndexes.back();
        m_freeIndexes.pop_back();
        m_alive[index] = true;
        return Entity(index, m_generations[index]);
    }
    m_generations.push_back(0);
    m_alive.push_back(true);
    return Entity(m_generations.size() - 1, 0);
}

Entity Registry::entityFromIndex(std::size_t index) const
{
    if (index >= m_alive.size() || !m_alive[index])
        throw EcsError("Registry::entityFromIndex: no living entity at this index");
    return Entity(index, m_generations[index]);
}

void Registry::killEntity(const Entity &entity)
{
    if (!isAlive(entity))
        return;
    for (auto &entry : m_components)
        entry.second->erase(entity.index());
    m_alive[entity.index()] = false;
    ++m_generations[entity.index()];
    m_freeIndexes.push_back(entity.index());
}

bool Registry::isAlive(const Entity &entity) const
{
    std::size_t index = entity.index();
    return index < m_alive.size() && m_alive[index] && m_generations[index] == entity.generation();
}

void Registry::checkAlive(const Entity &entity) const
{
    if (!isAlive(entity))
        throw EcsError("Registry: the entity is dead");
}

} // namespace engine::ecs
