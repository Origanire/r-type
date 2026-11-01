#include "PlayerSlot.hpp"

namespace rtype::server {

PlayerSlot::PlayerSlot(std::uint8_t lives) : m_lives(lives), m_score(0), m_respawnTimer(0.f)
{
}

std::uint8_t PlayerSlot::lives() const
{
    return m_lives;
}

std::uint32_t PlayerSlot::score() const
{
    return m_score;
}

float PlayerSlot::respawnTimer() const
{
    return m_respawnTimer;
}

void PlayerSlot::loseLife(float respawnDelay)
{
    if (m_lives > 0)
        --m_lives;
    m_respawnTimer = respawnDelay;
}

void PlayerSlot::addScore(std::uint32_t points)
{
    m_score += points;
}

void PlayerSlot::elapseRespawn(float dt)
{
    m_respawnTimer -= dt;
}

} // namespace rtype::server
