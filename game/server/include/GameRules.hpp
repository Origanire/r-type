#pragma once

#include <cstdint>
#include <string>

#include "DataNode.hpp"

namespace rtype::server {

class GameRules
{
public:
    explicit GameRules(const engine::scene::DataNode &rules);

    std::uint8_t startLives() const;
    float respawnDelay() const;
    float invulnerability() const;
    float gameOverDelay() const;
    float spawnX() const;
    float spawnY() const;
    float spawnSpacing() const;
    const std::string &playerPrefab() const;
    const std::string &spawnerPrefab() const;

private:
    std::uint8_t m_startLives;
    float m_respawnDelay;
    float m_invulnerability;
    float m_gameOverDelay;
    float m_spawnX;
    float m_spawnY;
    float m_spawnSpacing;
    std::string m_playerPrefab;
    std::string m_spawnerPrefab;
};

} // namespace rtype::server
