#include "EngineComponentLoaders.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

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
#include "Match.hpp"
#include "MovementPattern.hpp"
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

namespace {

ecs::MovementPattern::Kind readPatternKind(const DataNode &node)
{
    const std::string kind = node.getString("kind");
    if (kind == "straight")
        return ecs::MovementPattern::Kind::Straight;
    if (kind == "sine")
        return ecs::MovementPattern::Kind::Sine;
    if (kind == "zigzag")
        return ecs::MovementPattern::Kind::Zigzag;
    throw SceneError("ComponentLoaders: unknown movement pattern " + kind + " in " + node.path());
}

std::vector<ecs::SpawnEntry> readSpawnEntries(const DataNode &node)
{
    std::vector<ecs::SpawnEntry> entries;
    for (const DataNode &entry : node.child("entries").items())
        entries.emplace_back(entry.getString("prefab"), entry.getFloat("minX"), entry.getFloat("maxX"),
            entry.getFloat("minY"), entry.getFloat("maxY"), entry.getFloat("weight", 1.f));
    return entries;
}

std::optional<ecs::Match::Phase> readPhase(const DataNode &node, const std::string &key)
{
    if (!node.has(key))
        return std::nullopt;
    const std::string phase = node.getString(key);
    if (phase == "waiting")
        return ecs::Match::Phase::Waiting;
    if (phase == "playing")
        return ecs::Match::Phase::Playing;
    if (phase == "over")
        return ecs::Match::Phase::Over;
    throw SceneError("ComponentLoaders: unknown match phase " + phase + " in " + node.path());
}

std::optional<std::size_t> readSlot(const DataNode &node)
{
    if (!node.has("slot"))
        return std::nullopt;
    const int slot = node.getInt("slot");
    if (slot < 0)
        throw SceneError("ComponentLoaders: " + node.path() + ".slot must not be negative");
    return static_cast<std::size_t>(slot);
}

void addGameplayLoaders(ComponentFactory &factory, const LayerTable &layers)
{
    factory.add("Transform", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Transform>(entity, data.getFloat("x", 0.f), data.getFloat("y", 0.f));
    });
    factory.add("Velocity", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Velocity>(entity, data.getFloat("x", 0.f), data.getFloat("y", 0.f));
    });
    factory.add("Collider", [&layers](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        const std::uint32_t hits = data.has("hits") ? layers.mask(data.child("hits")) : 0;
        registry.emplace<ecs::Collider>(
            entity, data.getFloat("width"), data.getFloat("height"), layers.bit(data.getString("layer")), hits);
    });
    factory.add("Bounded", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Bounded>(
            entity, data.getFloat("minX"), data.getFloat("minY"), data.getFloat("maxX"), data.getFloat("maxY"));
    });
    factory.add("Health", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Health>(entity, data.getInt("points"));
    });
    factory.add("ContactDamage", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::ContactDamage>(entity, data.getInt("amount"), data.getBool("destroyOnHit", false));
    });
    factory.add("Invulnerable", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Invulnerable>(entity, data.getFloat("seconds"));
    });
    factory.add("MovementPattern", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::MovementPattern>(entity, readPatternKind(data), data.getFloat("amplitude", 0.f),
            data.getFloat("frequency", 0.f), data.getFloat("verticalSpeed", 0.f), data.getFloat("minY", 0.f),
            data.getFloat("maxY", 0.f));
    });
    factory.add("DespawnOffscreen", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::DespawnOffscreen>(
            entity, data.getFloat("minX"), data.getFloat("minY"), data.getFloat("maxX"), data.getFloat("maxY"));
    });
    factory.add("PlayerControl", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::PlayerControl>(entity, data.getFloat("speed"));
    });
    factory.add("ScoreValue", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::ScoreValue>(entity, static_cast<std::uint32_t>(data.getInt("points")));
    });
    factory.add("Weapon", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Weapon>(entity, data.getString("prefab"), data.getFloat("cooldown"),
            data.getFloat("offsetX", 0.f), data.getFloat("offsetY", 0.f));
    });
    factory.add("AutoFire", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::AutoFire>(entity, data.getString("prefab"), data.getFloat("minInterval"),
            data.getFloat("maxInterval"), data.getFloat("offsetX", 0.f), data.getFloat("offsetY", 0.f));
    });
    factory.add("Spawner", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Spawner>(
            entity, readSpawnEntries(data), data.getFloat("minInterval"), data.getFloat("maxInterval"));
    });
    factory.add("Replicated", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Replicated>(entity, static_cast<std::uint8_t>(data.getInt("kind")));
    });
}

void addRuleLoaders(ComponentFactory &factory)
{
    factory.add("Lives", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Lives>(entity, data.getInt("count"));
    });
    factory.add("Score", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &) {
        registry.emplace<ecs::Score>(entity);
    });
    factory.add("Respawn", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Respawn>(entity, data.getString("prefab"), data.getFloat("delay", 0.f),
            data.getFloat("invulnerability", 0.f), data.getFloat("x"), data.getFloat("y"),
            data.getFloat("spacingX", 0.f), data.getFloat("spacingY", 0.f));
    });
    factory.add("Match", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Match>(entity, data.getFloat("restartDelay", 0.f));
    });
    factory.add("Persistent", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &) {
        registry.emplace<ecs::Persistent>(entity);
    });
}

void addPresentationLoaders(ComponentFactory &factory)
{
    factory.add("Sprite", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Sprite>(
            entity, data.getString("texture"), readRect(data, "frame"), data.getFloat("scale", 1.f));
    });
    factory.add("RectShape", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::RectShape>(
            entity, data.getFloat("width"), data.getFloat("height"), readColor(data, "color"));
    });
    factory.add("Parallax", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Parallax>(entity, data.getFloat("speed"), data.getFloat("wrapWidth"));
    });
    factory.add("TiltAnimation", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::TiltAnimation>(entity, readRect(data, "frame"), data.getInt("row", 0),
            data.getInt("flatColumn"), data.getInt("maxTilt"), data.getFloat("framesPerSecond"));
    });
    factory.add("SpriteAnimation", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::SpriteAnimation>(entity, readRect(data, "frame"), data.getInt("frameCount"),
            data.getInt("stepX"), data.getFloat("framesPerSecond"), data.getBool("loop", false));
    });
    factory.add("SoundCue", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::SoundCue>(entity, data.getString("sound"));
    });
    factory.add("Text", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        const std::string content = data.has("content") ? data.getString("content") : std::string();
        registry.emplace<ecs::Text>(entity, data.getString("font"), content,
            static_cast<unsigned int>(data.getInt("size")), readColor(data, "color"));
    });
    factory.add("DeathEffect", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::DeathEffect>(entity, data.getString("prefab"));
    });
    factory.add("PlayerLabel", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::PlayerLabel>(entity, data.getString("font"), data.getString("format"),
            static_cast<unsigned int>(data.getInt("size")), data.getFloat("offsetX", 0.f),
            data.getFloat("offsetY", 0.f), readColors(data, "colors"));
    });
    factory.add("StatusText", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::StatusText>(
            entity, data.getString("format"), readSlot(data), readPhase(data, "visibleWhen"));
    });
}

} // namespace

void addEngineComponentLoaders(ComponentFactory &factory, const LayerTable &layers)
{
    addGameplayLoaders(factory, layers);
    addRuleLoaders(factory);
    addPresentationLoaders(factory);
}

} // namespace engine::scene
