#include "EngineSystemBuilders.hpp"

#include "AnchorSystem.hpp"
#include "AudioSystem.hpp"
#include "AutoFireSystem.hpp"
#include "BoundsSystem.hpp"
#include "CleanupSystem.hpp"
#include "CollisionSystem.hpp"
#include "Color.hpp"
#include "DamageSystem.hpp"
#include "DataReaders.hpp"
#include "DeathEffectSystem.hpp"
#include "InvulnerabilitySystem.hpp"
#include "LivesSystem.hpp"
#include "MatchSystem.hpp"
#include "MovementPatternSystem.hpp"
#include "MovementSystem.hpp"
#include "OffscreenSystem.hpp"
#include "ParallaxSystem.hpp"
#include "PlayerControlSystem.hpp"
#include "PlayerLabelSystem.hpp"
#include "RenderSystem.hpp"
#include "RespawnSystem.hpp"
#include "ScoreSystem.hpp"
#include "SpawnerSystem.hpp"
#include "SpriteAnimationSystem.hpp"
#include "StatusTextSystem.hpp"
#include "TiltAnimationSystem.hpp"
#include "WeaponSystem.hpp"

namespace engine::scene {

void EngineSystemBuilders::registerAll(SystemFactory &factory)
{
    registerSimulation(factory);
    registerRules(factory);
    registerPresentation(factory);
}

void EngineSystemBuilders::registerSimulation(SystemFactory &factory)
{
    factory.add("PlayerControl", &EngineSystemBuilders::buildSimple<ecs::PlayerControlSystem>);
    factory.add("MovementPattern", &EngineSystemBuilders::buildSimple<ecs::MovementPatternSystem>);
    factory.add("Invulnerability", &EngineSystemBuilders::buildSimple<ecs::InvulnerabilitySystem>);
    factory.add("Movement", &EngineSystemBuilders::buildSimple<ecs::MovementSystem>);
    factory.add("Bounds", &EngineSystemBuilders::buildSimple<ecs::BoundsSystem>);
    factory.add("Offscreen", &EngineSystemBuilders::buildSimple<ecs::OffscreenSystem>);
    factory.add("Cleanup", &EngineSystemBuilders::buildSimple<ecs::CleanupSystem>);
    factory.add("Weapon", &EngineSystemBuilders::buildSpawning<ecs::WeaponSystem>);
    factory.add("Spawner", &EngineSystemBuilders::buildSpawning<ecs::SpawnerSystem>);
    factory.add("AutoFire", &EngineSystemBuilders::buildSpawning<ecs::AutoFireSystem>);
    factory.add("Collision", &EngineSystemBuilders::buildCollision);
    factory.add("Damage", &EngineSystemBuilders::buildDamage);
}

void EngineSystemBuilders::registerRules(SystemFactory &factory)
{
    factory.add("Match", &EngineSystemBuilders::buildSimple<ecs::MatchSystem>);
    factory.add("Lives", &EngineSystemBuilders::buildDeathListener<ecs::LivesSystem>);
    factory.add("Score", &EngineSystemBuilders::buildDeathListener<ecs::ScoreSystem>);
    factory.add("Respawn", &EngineSystemBuilders::buildRespawn);
}

void EngineSystemBuilders::registerPresentation(SystemFactory &factory)
{
    factory.add("Parallax", &EngineSystemBuilders::buildSimple<ecs::ParallaxSystem>);
    factory.add("TiltAnimation", &EngineSystemBuilders::buildSimple<ecs::TiltAnimationSystem>);
    factory.add("SpriteAnimation", &EngineSystemBuilders::buildSimple<ecs::SpriteAnimationSystem>);
    factory.add("Anchor", &EngineSystemBuilders::buildSimple<ecs::AnchorSystem>);
    factory.add("StatusText", &EngineSystemBuilders::buildSimple<ecs::StatusTextSystem>);
    factory.add("DeathEffect", &EngineSystemBuilders::buildDeathEffect);
    factory.add("PlayerLabel", &EngineSystemBuilders::buildPlayerLabel);
    factory.add("Audio", &EngineSystemBuilders::buildAudio);
    factory.add("Render", &EngineSystemBuilders::buildRender);
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildCollision(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::CollisionSystem>(context.registry(), context.collisions());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildDamage(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::DamageSystem>(context.registry(), context.collisions(), context.deaths());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildRespawn(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::RespawnSystem>(context.registry(), context.factory());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildDeathEffect(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::DeathEffectSystem>(context.registry(), context.factory(), context.deaths());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildPlayerLabel(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::PlayerLabelSystem>(context.registry(), context.flag("showPlayerLabels", true));
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildAudio(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::AudioSystem>(context.registry(), context.audio());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildRender(SystemContext &context, const DataNode &entry)
{
    const Color background = entry.isString() ? Color(0, 0, 0) : DataReaders::readColor(entry, "background");
    return std::make_unique<ecs::RenderSystem>(context.registry(), context.renderer(), background);
}

} // namespace engine::scene
