#include "SpawnSystem.hpp"

#include <utility>

#include "EntityFactory.hpp"
#include "GameConstants.hpp"
#include "RtypeProtocol.hpp"
#include "Snapshot.hpp"

namespace rtype::server {

namespace {

constexpr double GOING_UP_CHANCE = 0.5;

} // namespace

SpawnSystem::SpawnSystem(
    ServerWorld &world, std::function<std::uint16_t()> nextNetworkId, std::function<bool()> canSpawn)
    : m_world(world), m_nextNetworkId(std::move(nextNetworkId)), m_canSpawn(std::move(canSpawn)),
      m_random(std::random_device{}()), m_timer(randomInterval())
{
}

void SpawnSystem::update(float dt)
{
    if (!m_canSpawn())
        return;
    m_timer -= dt;
    if (m_timer > 0.f)
        return;
    m_timer = randomInterval();
    if (countReplicated(m_world) >= protocol::MAX_ENTITIES)
        return;
    const auto pattern =
        static_cast<EnemyPattern>(std::uniform_int_distribution<int>(0, ENEMY_PATTERN_COUNT - 1)(m_random));
    const float margin = pattern == EnemyPattern::Sine ? BYDO_SINE_AMPLITUDE : 0.f;
    const float y = std::uniform_real_distribution<float>(margin, WORLD_HEIGHT - BYDO_HEIGHT - margin)(m_random);
    const bool goingUp = std::bernoulli_distribution(GOING_UP_CHANCE)(m_random);
    createBydo(m_world, Position{WORLD_WIDTH, y}, pattern, goingUp, m_nextNetworkId());
}

float SpawnSystem::randomInterval()
{
    return std::uniform_real_distribution<float>(BYDO_SPAWN_MIN_INTERVAL, BYDO_SPAWN_MAX_INTERVAL)(m_random);
}

} // namespace rtype::server
