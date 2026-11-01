#include "GameRules.hpp"

namespace rtype::server {

GameRules::GameRules(const engine::scene::DataNode &rules)
    : m_startLives(static_cast<std::uint8_t>(rules.getInt("startLives"))),
      m_respawnDelay(rules.getFloat("respawnDelay")), m_invulnerability(rules.getFloat("invulnerability")),
      m_gameOverDelay(rules.getFloat("gameOverDelay")), m_spawnX(rules.getFloat("spawnX")),
      m_spawnY(rules.getFloat("spawnY")), m_spawnSpacing(rules.getFloat("spawnSpacing")),
      m_playerPrefab(rules.getString("playerPrefab")), m_spawnerPrefab(rules.getString("spawnerPrefab"))
{
}

std::uint8_t GameRules::startLives() const
{
    return m_startLives;
}

float GameRules::respawnDelay() const
{
    return m_respawnDelay;
}

float GameRules::invulnerability() const
{
    return m_invulnerability;
}

float GameRules::gameOverDelay() const
{
    return m_gameOverDelay;
}

float GameRules::spawnX() const
{
    return m_spawnX;
}

float GameRules::spawnY() const
{
    return m_spawnY;
}

float GameRules::spawnSpacing() const
{
    return m_spawnSpacing;
}

const std::string &GameRules::playerPrefab() const
{
    return m_playerPrefab;
}

const std::string &GameRules::spawnerPrefab() const
{
    return m_spawnerPrefab;
}

} // namespace rtype::server
