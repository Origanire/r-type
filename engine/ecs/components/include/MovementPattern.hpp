#pragma once

#include <cstdint>

#include "IComponent.hpp"

namespace engine::ecs {

class MovementPattern : public IComponent
{
public:
    enum class Kind : std::uint8_t
    {
        Straight,
        Sine,
        Zigzag
    };

    MovementPattern(Kind kind, float amplitude, float frequency, float verticalSpeed, float minY, float maxY);

    Kind kind() const;
    float amplitude() const;
    float frequency() const;
    float verticalSpeed() const;
    float minY() const;
    float maxY() const;
    float age() const;
    void advance(float dt);
    bool hasBaseY() const;
    float baseY() const;
    void setBaseY(float y);

private:
    Kind m_kind;
    float m_amplitude;
    float m_frequency;
    float m_verticalSpeed;
    float m_minY;
    float m_maxY;
    float m_age;
    float m_baseY;
    bool m_hasBaseY;
};

} // namespace engine::ecs
