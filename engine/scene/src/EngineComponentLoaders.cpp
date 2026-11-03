#include "EngineComponentLoaders.hpp"

#include <cstdint>
#include <functional>

#include "AutoFire.hpp"
#include "Bounded.hpp"
#include "Collider.hpp"
#include "ContactDamage.hpp"
#include "DataReaders.hpp"
#include "DeathEffect.hpp"
#include "DespawnOffscreen.hpp"
#include "Health.hpp"
#include "Invulnerable.hpp"
#include "Lives.hpp"
#include "Parallax.hpp"
#include "Persistent.hpp"
#include "PlayerControl.hpp"
#include "PlayerLabel.hpp"
#include "RectShape.hpp"
#include "Replicated.hpp"
#include "Respawn.hpp"
#include "SceneError.hpp"
#include "Score.hpp"
#include "ScoreValue.hpp"
#include "SoundCue.hpp"
#include "Spawner.hpp"
#include "Sprite.hpp"
#include "SpriteAnimation.hpp"
#include "StatusText.hpp"
#include "Text.hpp"
#include "TiltAnimation.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "Weapon.hpp"

namespace engine::scene {

void EngineComponentLoaders::registerAll(ComponentFactory &factory, const LayerTable &layers)
{
    registerGameplay(factory, layers);
    registerRules(factory);
    registerPresentation(factory);
}

void EngineComponentLoaders::registerGameplay(ComponentFactory &factory, const LayerTable &layers)
{
    factory.add("Transform", &EngineComponentLoaders::loadTransform);
    factory.add("Velocity", &EngineComponentLoaders::loadVelocity);
    factory.add("Collider", std::bind_front(&EngineComponentLoaders::loadCollider, std::cref(layers)));
    factory.add("Bounded", &EngineComponentLoaders::loadBounded);
    factory.add("Health", &EngineComponentLoaders::loadHealth);
    factory.add("ContactDamage", &EngineComponentLoaders::loadContactDamage);
    factory.add("Invulnerable", &EngineComponentLoaders::loadInvulnerable);
    factory.add("MovementPattern", &EngineComponentLoaders::loadMovementPattern);
    factory.add("DespawnOffscreen", &EngineComponentLoaders::loadDespawnOffscreen);
    factory.add("PlayerControl", &EngineComponentLoaders::loadPlayerControl);
    factory.add("ScoreValue", &EngineComponentLoaders::loadScoreValue);
    factory.add("Weapon", &EngineComponentLoaders::loadWeapon);
    factory.add("AutoFire", &EngineComponentLoaders::loadAutoFire);
    factory.add("Spawner", &EngineComponentLoaders::loadSpawner);
    factory.add("Replicated", &EngineComponentLoaders::loadReplicated);
}

void EngineComponentLoaders::registerRules(ComponentFactory &factory)
{
    factory.add("Lives", &EngineComponentLoaders::loadLives);
    factory.add("Score", &EngineComponentLoaders::loadScore);
    factory.add("Respawn", &EngineComponentLoaders::loadRespawn);
    factory.add("Match", &EngineComponentLoaders::loadMatch);
    factory.add("Persistent", &EngineComponentLoaders::loadPersistent);
}

void EngineComponentLoaders::registerPresentation(ComponentFactory &factory)
{
    factory.add("Sprite", &EngineComponentLoaders::loadSprite);
    factory.add("RectShape", &EngineComponentLoaders::loadRectShape);
    factory.add("Parallax", &EngineComponentLoaders::loadParallax);
    factory.add("TiltAnimation", &EngineComponentLoaders::loadTiltAnimation);
    factory.add("SpriteAnimation", &EngineComponentLoaders::loadSpriteAnimation);
    factory.add("SoundCue", &EngineComponentLoaders::loadSoundCue);
    factory.add("Text", &EngineComponentLoaders::loadText);
    factory.add("DeathEffect", &EngineComponentLoaders::loadDeathEffect);
    factory.add("PlayerLabel", &EngineComponentLoaders::loadPlayerLabel);
    factory.add("StatusText", &EngineComponentLoaders::loadStatusText);
}

void EngineComponentLoaders::loadTransform(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Transform>(entity, data.getFloat("x", 0.f), data.getFloat("y", 0.f));
}

void EngineComponentLoaders::loadVelocity(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Velocity>(entity, data.getFloat("x", 0.f), data.getFloat("y", 0.f));
}

void EngineComponentLoaders::loadCollider(
    const LayerTable &layers, ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    const std::uint32_t hits = data.has("hits") ? layers.mask(data.child("hits")) : 0;
    registry.emplace<ecs::Collider>(
        entity, data.getFloat("width"), data.getFloat("height"), layers.bit(data.getString("layer")), hits);
}

void EngineComponentLoaders::loadBounded(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Bounded>(
        entity, data.getFloat("minX"), data.getFloat("minY"), data.getFloat("maxX"), data.getFloat("maxY"));
}

void EngineComponentLoaders::loadHealth(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Health>(entity, data.getInt("points"));
}

void EngineComponentLoaders::loadContactDamage(
    ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::ContactDamage>(entity, data.getInt("amount"), data.getBool("destroyOnHit", false));
}

void EngineComponentLoaders::loadInvulnerable(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Invulnerable>(entity, data.getFloat("seconds"));
}

void EngineComponentLoaders::loadMovementPattern(
    ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::MovementPattern>(entity, readPatternKind(data), data.getFloat("amplitude", 0.f),
        data.getFloat("frequency", 0.f), data.getFloat("verticalSpeed", 0.f), data.getFloat("minY", 0.f),
        data.getFloat("maxY", 0.f));
}

void EngineComponentLoaders::loadDespawnOffscreen(
    ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::DespawnOffscreen>(
        entity, data.getFloat("minX"), data.getFloat("minY"), data.getFloat("maxX"), data.getFloat("maxY"));
}

void EngineComponentLoaders::loadPlayerControl(
    ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::PlayerControl>(entity, data.getFloat("speed"));
}

void EngineComponentLoaders::loadScoreValue(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::ScoreValue>(entity, static_cast<std::uint32_t>(data.getInt("points")));
}

void EngineComponentLoaders::loadWeapon(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Weapon>(entity, data.getString("prefab"), data.getFloat("cooldown"),
        data.getFloat("offsetX", 0.f), data.getFloat("offsetY", 0.f));
}

void EngineComponentLoaders::loadAutoFire(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::AutoFire>(entity, data.getString("prefab"), data.getFloat("minInterval"),
        data.getFloat("maxInterval"), data.getFloat("offsetX", 0.f), data.getFloat("offsetY", 0.f));
}

void EngineComponentLoaders::loadSpawner(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Spawner>(
        entity, readSpawnEntries(data), data.getFloat("minInterval"), data.getFloat("maxInterval"));
}

void EngineComponentLoaders::loadReplicated(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Replicated>(entity, static_cast<std::uint8_t>(data.getInt("kind")));
}

void EngineComponentLoaders::loadLives(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Lives>(entity, data.getInt("count"));
}

void EngineComponentLoaders::loadScore(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Score>(entity);
}

void EngineComponentLoaders::loadRespawn(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Respawn>(entity, data.getString("prefab"), data.getFloat("delay", 0.f),
        data.getFloat("invulnerability", 0.f), data.getFloat("x"), data.getFloat("y"), data.getFloat("spacingX", 0.f),
        data.getFloat("spacingY", 0.f));
}

void EngineComponentLoaders::loadMatch(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Match>(entity, data.getFloat("restartDelay", 0.f));
}

void EngineComponentLoaders::loadPersistent(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Persistent>(entity);
}

void EngineComponentLoaders::loadSprite(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Sprite>(
        entity, data.getString("texture"), DataReaders::readRect(data, "frame"), data.getFloat("scale", 1.f));
}

void EngineComponentLoaders::loadRectShape(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::RectShape>(
        entity, data.getFloat("width"), data.getFloat("height"), DataReaders::readColor(data, "color"));
}

void EngineComponentLoaders::loadParallax(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Parallax>(entity, data.getFloat("speed"), data.getFloat("wrapWidth"));
}

void EngineComponentLoaders::loadTiltAnimation(
    ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::TiltAnimation>(entity, DataReaders::readRect(data, "frame"), data.getInt("row", 0),
        data.getInt("flatColumn"), data.getInt("maxTilt"), data.getFloat("framesPerSecond"));
}

void EngineComponentLoaders::loadSpriteAnimation(
    ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::SpriteAnimation>(entity, DataReaders::readRect(data, "frame"), data.getInt("frameCount"),
        data.getInt("stepX"), data.getFloat("framesPerSecond"), data.getBool("loop", false));
}

void EngineComponentLoaders::loadSoundCue(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::SoundCue>(entity, data.getString("sound"));
}

void EngineComponentLoaders::loadText(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    const std::string content = data.has("content") ? data.getString("content") : std::string();
    registry.emplace<ecs::Text>(entity, data.getString("font"), content,
        static_cast<unsigned int>(data.getInt("size")), DataReaders::readColor(data, "color"));
}

void EngineComponentLoaders::loadDeathEffect(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::DeathEffect>(entity, data.getString("prefab"));
}

void EngineComponentLoaders::loadPlayerLabel(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::PlayerLabel>(entity, data.getString("font"), data.getString("format"),
        static_cast<unsigned int>(data.getInt("size")), data.getFloat("offsetX", 0.f), data.getFloat("offsetY", 0.f),
        DataReaders::readColors(data, "colors"));
}

void EngineComponentLoaders::loadStatusText(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::StatusText>(entity, data.getString("format"), readSlot(data), readPhase(data, "visibleWhen"));
}

ecs::MovementPattern::Kind EngineComponentLoaders::readPatternKind(const DataNode &data)
{
    const std::string kind = data.getString("kind");
    if (kind == "straight")
        return ecs::MovementPattern::Kind::Straight;
    if (kind == "sine")
        return ecs::MovementPattern::Kind::Sine;
    if (kind == "zigzag")
        return ecs::MovementPattern::Kind::Zigzag;
    throw SceneError("EngineComponentLoaders: unknown movement pattern " + kind + " in " + data.path());
}

std::vector<ecs::SpawnEntry> EngineComponentLoaders::readSpawnEntries(const DataNode &data)
{
    std::vector<ecs::SpawnEntry> entries;
    for (const DataNode &entry : data.child("entries").items())
        entries.emplace_back(entry.getString("prefab"), entry.getFloat("minX"), entry.getFloat("maxX"),
            entry.getFloat("minY"), entry.getFloat("maxY"), entry.getFloat("weight", 1.f));
    return entries;
}

std::optional<ecs::Match::Phase> EngineComponentLoaders::readPhase(const DataNode &data, const std::string &key)
{
    if (!data.has(key))
        return std::nullopt;
    const std::string phase = data.getString(key);
    if (phase == "waiting")
        return ecs::Match::Phase::Waiting;
    if (phase == "playing")
        return ecs::Match::Phase::Playing;
    if (phase == "over")
        return ecs::Match::Phase::Over;
    throw SceneError("EngineComponentLoaders: unknown match phase " + phase + " in " + data.path());
}

std::optional<std::size_t> EngineComponentLoaders::readSlot(const DataNode &data)
{
    if (!data.has("slot"))
        return std::nullopt;
    const int slot = data.getInt("slot");
    if (slot < 0)
        throw SceneError("EngineComponentLoaders: " + data.path() + ".slot must not be negative");
    return static_cast<std::size_t>(slot);
}

} // namespace engine::scene
