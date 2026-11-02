#pragma once

#include <string>

namespace engine::ecs {

class SpawnEntry
{
public:
    SpawnEntry(std::string prefab, float minX, float maxX, float minY, float maxY, float weight);

    const std::string &prefab() const;
    float minX() const;
    float maxX() const;
    float minY() const;
    float maxY() const;
    float weight() const;

private:
    std::string m_prefab;
    float m_minX;
    float m_maxX;
    float m_minY;
    float m_maxY;
    float m_weight;
};

} // namespace engine::ecs
