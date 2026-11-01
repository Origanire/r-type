#pragma once

#include <cstdint>

#include "IComponent.hpp"

namespace engine::ecs {

class Collider : public IComponent
{
public:
    Collider(float width, float height, std::uint32_t layer, std::uint32_t hitMask);

    float width() const;
    float height() const;
    std::uint32_t layer() const;
    std::uint32_t hitMask() const;
    bool canHit(const Collider &other) const;

private:
    float m_width;
    float m_height;
    std::uint32_t m_layer;
    std::uint32_t m_hitMask;
};

} // namespace engine::ecs
