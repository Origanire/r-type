#include "EngineComponentLoaders.hpp"

#include <cstdint>
#include <functional>
#include <utility>

#include "BoundText.hpp"
#include "Bounded.hpp"
#include "Button.hpp"
#include "Collider.hpp"
#include "ContactDamage.hpp"
#include "DataReaders.hpp"
#include "DeathEffect.hpp"
#include "DespawnOffscreen.hpp"
#include "FadeWhen.hpp"
#include "Focusable.hpp"
#include "Foreground.hpp"
#include "Health.hpp"
#include "Hidden.hpp"
#include "Invulnerable.hpp"
#include "ListView.hpp"
#include "Lives.hpp"
#include "Parallax.hpp"
#include "Persistent.hpp"
#include "PlayerControl.hpp"
#include "PlayerLabel.hpp"
#include "ProgressBar.hpp"
#include "RectShape.hpp"
#include "Replicated.hpp"
#include "Respawn.hpp"
#include "SceneError.hpp"
#include "Score.hpp"
#include "Script.hpp"
#include "Bounce.hpp"
#include "Choice.hpp"
#include "Gravity.hpp"
#include "Lifetime.hpp"
#include "MusicWhen.hpp"
#include "ParticleEmitter.hpp"
#include "Pushable.hpp"
#include "Rotation.hpp"
#include "Solid.hpp"
#include "Tilemap.hpp"
#include "ScoreValue.hpp"
#include "SoundCue.hpp"
#include "Sprite.hpp"
#include "SpriteAnimation.hpp"
#include "StatusText.hpp"
#include "Text.hpp"
#include "TextField.hpp"
#include "TiltAnimation.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "VisibleWhen.hpp"
#include "Weapon.hpp"

namespace engine::scene {

void EngineComponentLoaders::registerAll(ComponentFactory &factory, const LayerTable &layers)
{
    registerGameplay(factory, layers);
    registerRules(factory);
    registerPresentation(factory);
    registerInterface(factory);
}

void EngineComponentLoaders::registerInterface(ComponentFactory &factory)
{
    factory.add("Button", &EngineComponentLoaders::loadButton);
    factory.add("Focusable", &EngineComponentLoaders::loadFocusable);
    factory.add("TextField", &EngineComponentLoaders::loadTextField);
    factory.add("VisibleWhen", &EngineComponentLoaders::loadVisibleWhen);
    factory.add("BoundText", &EngineComponentLoaders::loadBoundText);
    factory.add("ListView", &EngineComponentLoaders::loadListView);
    factory.add("Hidden", &EngineComponentLoaders::loadHidden);
    factory.add("Foreground", &EngineComponentLoaders::loadForeground);
    factory.add("ProgressBar", &EngineComponentLoaders::loadProgressBar);
    factory.add("FadeWhen", &EngineComponentLoaders::loadFadeWhen);
}

void EngineComponentLoaders::loadButton(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    ecs::Button &button =
        registry.emplace<ecs::Button>(entity, data.getString("action"), data.getFloat("width"), data.getFloat("height"));
    if (data.has("normal"))
        button.setColors(DataReaders::readColor(data, "normal"), DataReaders::readColor(data, "hover"),
            DataReaders::readColor(data, "pressed"));
    if (data.has("normalFrame"))
        button.setFrames(DataReaders::readRect(data, "normalFrame"), DataReaders::readRect(data, "hoverFrame"),
            DataReaders::readRect(data, "pressedFrame"));
    button.setSounds(readOptionalString(data, "hoverSound"), readOptionalString(data, "clickSound"));
}

void EngineComponentLoaders::loadFocusable(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Focusable>(entity, data.getInt("order"));
}

void EngineComponentLoaders::loadTextField(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    const int maxLength = data.getInt("maxLength");
    if (maxLength <= 0)
        throw SceneError("EngineComponentLoaders: " + data.path() + ".maxLength must be positive");
    registry.emplace<ecs::TextField>(entity, data.getString("name"), static_cast<std::size_t>(maxLength),
        data.getBool("masked", false), readOptionalString(data, "placeholder"), data.getFloat("width"),
        data.getFloat("height"));
}

void EngineComponentLoaders::loadVisibleWhen(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::VisibleWhen>(entity, data.getString("key"), data.getString("value"));
}

void EngineComponentLoaders::loadBoundText(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::BoundText>(entity, data.getString("format"));
}

void EngineComponentLoaders::loadListView(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    std::vector<ecs::ListRowPart> parts;
    for (const DataNode &part : data.child("parts").items())
        parts.emplace_back(part.getString("prefab"), part.getFloat("offsetX", 0.f), part.getFloat("offsetY", 0.f));
    const int maxRows = data.getInt("maxRows");
    if (maxRows <= 0)
        throw SceneError("EngineComponentLoaders: " + data.path() + ".maxRows must be positive");
    registry.emplace<ecs::ListView>(
        entity, data.getString("source"), data.getFloat("spacing"), static_cast<std::size_t>(maxRows), std::move(parts));
}

void EngineComponentLoaders::loadHidden(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Hidden>(entity);
}

void EngineComponentLoaders::loadForeground(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Foreground>(entity);
}

void EngineComponentLoaders::loadProgressBar(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::ProgressBar>(
        entity, data.getString("key"), data.getFloat("width"), data.getFloat("max", 1.f), data.getBool("vertical", false));
}

void EngineComponentLoaders::loadFadeWhen(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::FadeWhen>(entity, data.getString("key"), data.getString("value"), data.getFloat("seconds", 0.f));
}

std::string EngineComponentLoaders::readOptionalString(const DataNode &data, const std::string &key)
{
    return data.has(key) ? data.getString(key) : std::string();
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
    factory.add("DespawnOffscreen", &EngineComponentLoaders::loadDespawnOffscreen);
    factory.add("PlayerControl", &EngineComponentLoaders::loadPlayerControl);
    factory.add("ScoreValue", &EngineComponentLoaders::loadScoreValue);
    factory.add("Weapon", &EngineComponentLoaders::loadWeapon);
    factory.add("Replicated", &EngineComponentLoaders::loadReplicated);
    factory.add("Script", &EngineComponentLoaders::loadScript);
    factory.add("Solid", &EngineComponentLoaders::loadSolid);
    factory.add("Gravity", &EngineComponentLoaders::loadGravity);
    factory.add("Bounce", &EngineComponentLoaders::loadBounce);
    factory.add("Pushable", &EngineComponentLoaders::loadPushable);
    factory.add("Tilemap", &EngineComponentLoaders::loadTilemap);
    factory.add("ParticleEmitter", &EngineComponentLoaders::loadParticleEmitter);
    factory.add("Lifetime", &EngineComponentLoaders::loadLifetime);
    factory.add("MusicWhen", &EngineComponentLoaders::loadMusicWhen);
    factory.add("Choice", &EngineComponentLoaders::loadChoice);
    factory.add("Rotation", &EngineComponentLoaders::loadRotation);
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
        data.getFloat("offsetX", 0.f), data.getFloat("offsetY", 0.f), data.getInt("count", 1),
        data.getFloat("spread", 0.f));
}

void EngineComponentLoaders::loadScript(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    ecs::Script::Numbers numbers;
    ecs::Script::Texts texts;
    if (data.has("params")) {
        const DataNode params = data.child("params");
        for (const std::string &name : params.keys()) {
            const DataNode value = params.child(name);
            if (value.isString())
                texts.emplace(name, value.asString());
            else
                numbers.emplace(name, value.asFloat());
        }
    }
    registry.emplace<ecs::Script>(entity, data.getString("file"), std::move(numbers), std::move(texts));
}

void EngineComponentLoaders::loadSolid(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Solid>(entity);
}

void EngineComponentLoaders::loadGravity(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Gravity>(entity, data.getFloat("force"));
}

void EngineComponentLoaders::loadBounce(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Bounce>(entity, data.getFloat("restitution", 1.f));
}

void EngineComponentLoaders::loadPushable(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Pushable>(entity);
}

void EngineComponentLoaders::loadTilemap(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    std::vector<std::string> rows;
    for (const DataNode &row : data.child("rows").items())
        rows.push_back(row.asString());
    ecs::Tilemap::Legend legend;
    const DataNode tiles = data.child("legend");
    for (const std::string &symbol : tiles.keys()) {
        if (symbol.size() != 1)
            throw SceneError("Tilemap: legend keys must be single characters, got " + symbol);
        legend.emplace(symbol[0], tiles.getString(symbol));
    }
    registry.emplace<ecs::Tilemap>(
        entity, std::move(rows), std::move(legend), data.getFloat("tileSize"), data.getFloat("speed"));
}

void EngineComponentLoaders::loadParticleEmitter(
    ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::ParticleEmitter>(entity, data.getInt("count"), data.getFloat("speed"),
        data.getFloat("lifetime"), data.getFloat("size", 4.f), DataReaders::readColor(data, "color"));
}

void EngineComponentLoaders::loadLifetime(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Lifetime>(entity, data.getFloat("seconds"));
}

void EngineComponentLoaders::loadMusicWhen(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::MusicWhen>(entity, data.getString("key"), data.getString("value"), data.getString("music"));
}

void EngineComponentLoaders::loadChoice(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    std::vector<std::string> options;
    for (const DataNode &option : data.child("options").items())
        options.push_back(option.asString());
    registry.emplace<ecs::Choice>(entity, data.getString("name"), std::move(options));
}

void EngineComponentLoaders::loadRotation(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Rotation>(entity, data.getFloat("degrees", 0.f));
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
    ecs::Text &text = registry.emplace<ecs::Text>(entity, data.getString("font"), readOptionalString(data, "content"),
        static_cast<unsigned int>(data.getInt("size")), DataReaders::readColor(data, "color"));
    text.setOffset(data.getFloat("offsetX", 0.f), data.getFloat("offsetY", 0.f));
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
