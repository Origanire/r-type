#pragma once

#include <cstdint>

namespace rtype::server {

class PlayerSlot
{
public:
    explicit PlayerSlot(std::uint8_t lives);

    std::uint8_t lives() const;
    std::uint32_t score() const;
    float respawnTimer() const;
    void loseLife(float respawnDelay);
    void addScore(std::uint32_t points);
    void elapseRespawn(float dt);

private:
    std::uint8_t m_lives;
    std::uint32_t m_score;
    float m_respawnTimer;
};

} // namespace rtype::server
