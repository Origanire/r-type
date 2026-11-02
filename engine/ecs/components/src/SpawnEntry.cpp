#include "SpawnEntry.hpp"

#include <utility>

namespace engine::ecs {

SpawnEntry::SpawnEntry(std::string prefab, float minX, float maxX, float minY, float maxY, float weight)
    : m_prefab(std::move(prefab)), m_minX(minX), m_maxX(maxX), m_minY(minY), m_maxY(maxY), m_weight(weight)
{
}

const std::string &SpawnEntry::prefab() const
{
    return m_prefab;
}

float SpawnEntry::minX() const
{
    return m_minX;
}

float SpawnEntry::maxX() const
{
    return m_maxX;
}

float SpawnEntry::minY() const
{
    return m_minY;
}

float SpawnEntry::maxY() const
{
    return m_maxY;
}

float SpawnEntry::weight() const
{
    return m_weight;
}

} // namespace engine::ecs
