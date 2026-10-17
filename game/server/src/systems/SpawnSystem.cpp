#include "SpawnSystem.hpp"

#include <utility>

#include "CommandBuffer.hpp"
#include "Enemy.hpp"
#include "EntityFactory.hpp"
#include "GameConstants.hpp"
#include "Position.hpp"
#include "Registry.hpp"
#include "RtypeProtocol.hpp"
#include "SnapshotSystem.hpp"

namespace rtype::server {

SpawnSystem::SpawnSystem(std::function<std::uint16_t()> nextNetworkId)
    : m_nextNetworkId(std::move(nextNetworkId)), m_random(std::random_device{}()), m_timer(randomInterval())
{
}

void SpawnSystem::operator()(engine::ecs::SystemContext &context)
{
    m_timer -= context.dt;
    if (m_timer > 0.f)
        return;
    m_timer = randomInterval();
    // Snapshots must stay complete: skip this Bydo rather than exceed the entity limit.
    if (countReplicated(context.registry) >= protocol::MAX_ENTITIES)
        return;
    const auto pattern =
        static_cast<EnemyPattern>(std::uniform_int_distribution<int>(0, ENEMY_PATTERN_COUNT - 1)(m_random));
    // A sine Bydo keeps its whole wave inside the world.
    const float margin = pattern == EnemyPattern::Sine ? BYDO_SINE_AMPLITUDE : 0.f;
    const float y = std::uniform_real_distribution<float>(margin, WORLD_HEIGHT - BYDO_HEIGHT - margin)(m_random);
    const bool goingUp = std::bernoulli_distribution(0.5)(m_random);
    const Position origin{WORLD_WIDTH, y};
    const std::uint16_t networkId = m_nextNetworkId();
    context.commands.spawn(
        [origin, pattern, goingUp, networkId](engine::ecs::Registry &registry, const engine::ecs::Entity &bydo) {
            createBydo(registry, bydo, origin, pattern, goingUp, networkId);
        });
}

float SpawnSystem::randomInterval()
{
    return std::uniform_real_distribution<float>(BYDO_SPAWN_MIN_INTERVAL, BYDO_SPAWN_MAX_INTERVAL)(m_random);
}

} // namespace rtype::server
