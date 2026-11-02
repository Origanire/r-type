#include "EngineSystemBuilders.hpp"

#include <memory>

#include "AnchorSystem.hpp"
#include "AudioSystem.hpp"
#include "AutoFireSystem.hpp"
#include "BoundsSystem.hpp"
#include "CleanupSystem.hpp"
#include "CollisionSystem.hpp"
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

namespace {

template <class System>
void addSimple(SystemFactory &factory, const std::string &name)
{
    factory.add(name, [](SystemContext &context, const DataNode &) -> std::unique_ptr<ecs::ISystem> {
        return std::make_unique<System>(context.registry());
    });
}

template <class System>
void addSpawning(SystemFactory &factory, const std::string &name)
{
    factory.add(name, [](SystemContext &context, const DataNode &) -> std::unique_ptr<ecs::ISystem> {
        return std::make_unique<System>(context.registry(), context.factory(), context.canSpawn());
    });
}

template <class System>
void addDeathListener(SystemFactory &factory, const std::string &name)
{
    factory.add(name, [](SystemContext &context, const DataNode &) -> std::unique_ptr<ecs::ISystem> {
        return std::make_unique<System>(context.registry(), context.deaths());
    });
}

} // namespace

void addEngineSystemBuilders(SystemFactory &factory)
{
    addSimple<ecs::PlayerControlSystem>(factory, "PlayerControl");
    addSimple<ecs::MovementPatternSystem>(factory, "MovementPattern");
    addSimple<ecs::InvulnerabilitySystem>(factory, "Invulnerability");
    addSimple<ecs::MovementSystem>(factory, "Movement");
    addSimple<ecs::BoundsSystem>(factory, "Bounds");
    addSimple<ecs::OffscreenSystem>(factory, "Offscreen");
    addSimple<ecs::CleanupSystem>(factory, "Cleanup");
    addSimple<ecs::ParallaxSystem>(factory, "Parallax");
    addSimple<ecs::TiltAnimationSystem>(factory, "TiltAnimation");
    addSimple<ecs::SpriteAnimationSystem>(factory, "SpriteAnimation");
    addSimple<ecs::AnchorSystem>(factory, "Anchor");
    addSimple<ecs::MatchSystem>(factory, "Match");
    addSimple<ecs::StatusTextSystem>(factory, "StatusText");
    addSpawning<ecs::WeaponSystem>(factory, "Weapon");
    addSpawning<ecs::SpawnerSystem>(factory, "Spawner");
    addSpawning<ecs::AutoFireSystem>(factory, "AutoFire");
    addDeathListener<ecs::LivesSystem>(factory, "Lives");
    addDeathListener<ecs::ScoreSystem>(factory, "Score");
    factory.add("Collision", [](SystemContext &context, const DataNode &) -> std::unique_ptr<ecs::ISystem> {
        return std::make_unique<ecs::CollisionSystem>(context.registry(), context.collisions());
    });
    factory.add("Damage", [](SystemContext &context, const DataNode &) -> std::unique_ptr<ecs::ISystem> {
        return std::make_unique<ecs::DamageSystem>(context.registry(), context.collisions(), context.deaths());
    });
    factory.add("Respawn", [](SystemContext &context, const DataNode &) -> std::unique_ptr<ecs::ISystem> {
        return std::make_unique<ecs::RespawnSystem>(context.registry(), context.factory());
    });
    factory.add("DeathEffect", [](SystemContext &context, const DataNode &) -> std::unique_ptr<ecs::ISystem> {
        return std::make_unique<ecs::DeathEffectSystem>(context.registry(), context.factory(), context.deaths());
    });
    factory.add("PlayerLabel", [](SystemContext &context, const DataNode &) -> std::unique_ptr<ecs::ISystem> {
        return std::make_unique<ecs::PlayerLabelSystem>(context.registry(), context.flag("showPlayerLabels", true));
    });
    factory.add("Audio", [](SystemContext &context, const DataNode &) -> std::unique_ptr<ecs::ISystem> {
        return std::make_unique<ecs::AudioSystem>(context.registry(), context.audio());
    });
    factory.add("Render", [](SystemContext &context, const DataNode &entry) -> std::unique_ptr<ecs::ISystem> {
        const Color background = entry.isString() ? Color(0, 0, 0) : readColor(entry, "background");
        return std::make_unique<ecs::RenderSystem>(context.registry(), context.renderer(), background);
    });
}

} // namespace engine::scene
