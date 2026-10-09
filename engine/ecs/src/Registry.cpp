#include "Registry.hpp"

namespace engine::ecs {

Entity Registry::spawnEntity()
{
    if (!m_freeIndexes.empty()) {
        std::size_t idx = m_freeIndexes.back();
        m_freeIndexes.pop_back();
        m_alive[idx] = true;
        return Entity(idx, m_generations[idx]);
    }
    m_generations.push_back(0);
    m_alive.push_back(true);
    return Entity(m_generations.size() - 1, 0);
}

Entity Registry::entityFromIndex(std::size_t idx) const
{
    if (idx >= m_alive.size() || !m_alive[idx])
        throw EcsError("Registry::entityFromIndex: no living entity at this index");
    return Entity(idx, m_generations[idx]);
}

void Registry::killEntity(const Entity &entity)
{
    if (!isAlive(entity))
        return;
    for (auto &erase : m_eraseFunctions)
        erase(*this, entity);
    m_alive[entity.index()] = false;
    ++m_generations[entity.index()];
    m_freeIndexes.push_back(entity.index());
}

bool Registry::isAlive(const Entity &entity) const
{
    std::size_t idx = entity.index();
    return idx < m_alive.size() && m_alive[idx] && m_generations[idx] == entity.generation();
}

void Registry::checkAlive(const Entity &entity) const
{
    if (!isAlive(entity))
        throw EcsError("Registry: the entity is dead");
}

} // namespace engine::ecs
