#include "HitFlash.hpp"

namespace engine::ecs {

HitFlash::HitFlash(float seconds) : m_remaining(seconds)
{
}

float HitFlash::remaining() const
{
    return m_remaining;
}

void HitFlash::restart(float seconds)
{
    m_remaining = seconds;
}

void HitFlash::elapse(float dt)
{
    m_remaining -= dt;
}

} // namespace engine::ecs
