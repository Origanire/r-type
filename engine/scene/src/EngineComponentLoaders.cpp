#include "EngineComponentLoaders.hpp"

#include <cstdint>
#include <vector>

#include "AutoFire.hpp"
#include "Bounded.hpp"
#include "Collider.hpp"
#include "ContactDamage.hpp"
#include "DespawnOffscreen.hpp"
#include "Health.hpp"
#include "Invulnerable.hpp"
#include "MovementPattern.hpp"
#include "Parallax.hpp"
#include "PlayerControl.hpp"
#include "RectShape.hpp"
#include "Color.hpp"
#include "IntRect.hpp"
#include "Replicated.hpp"
#include "SceneError.hpp"
#include "ScoreValue.hpp"
#include "SoundCue.hpp"
#include "Spawner.hpp"
#include "Sprite.hpp"
#include "SpriteAnimation.hpp"
#include "Text.hpp"
#include "TiltAnimation.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "Weapon.hpp"

namespace engine::scene {

namespace {

constexpr std::size_t RECT_VALUES = 4;
constexpr std::size_t COLOR_VALUES = 4;

IntRect readRect(const DataNode &node, const std::string &key)
{
    const std::vector<DataNode> values = node.child(key).items();
    if (values.size() != RECT_VALUES)
        throw SceneError("ComponentLoaders: " + node.path() + "." + key + " must be [x, y, width, height]");
    return {values[0].asInt(), values[1].asInt(), values[2].asInt(), values[3].asInt()};
}

Color readColor(const DataNode &node, const std::string &key)
{
    const std::vector<DataNode> values = node.child(key).items();
    if (values.size() != COLOR_VALUES)
        throw SceneError("ComponentLoaders: " + node.path() + "." + key + " must be [r, g, b, a]");
    return {static_cast<std::uint8_t>(values[0].asInt()), static_cast<std::uint8_t>(values[1].asInt()),
        static_cast<std::uint8_t>(values[2].asInt()), static_cast<std::uint8_t>(values[3].asInt())};
}

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
        entries.emplace_back(entry.getString("prefab"), entry.getFloat("minY"), entry.getFloat("maxY"),
            entry.getFloat("weight", 1.f));
    return entries;
}

} // namespace

void addEngineComponentLoaders(ComponentFactory &factory, const LayerTable &layers)
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
    factory.add("Spawner", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Spawner>(entity, readSpawnEntries(data), data.getFloat("minInterval"),
            data.getFloat("maxInterval"), data.getFloat("x"));
    });
    factory.add("Replicated", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Replicated>(entity, static_cast<std::uint8_t>(data.getInt("kind")));
    });
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
    factory.add("AutoFire", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::AutoFire>(entity, data.getString("prefab"), data.getFloat("minInterval"),
            data.getFloat("maxInterval"), data.getFloat("offsetX", 0.f), data.getFloat("offsetY", 0.f));
    });
    factory.add("SpriteAnimation", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::SpriteAnimation>(entity, readRect(data, "frame"), data.getInt("frameCount"),
            data.getInt("stepX"), data.getFloat("framesPerSecond"), data.getBool("loop", false));
    });
    factory.add("SoundCue", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::SoundCue>(entity, data.getString("sound"));
    });
    factory.add("Text", [](ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data) {
        registry.emplace<ecs::Text>(entity, data.getString("font"), data.getString("content"),
            static_cast<unsigned int>(data.getInt("size")), readColor(data, "color"));
    });
}

} // namespace engine::scene
