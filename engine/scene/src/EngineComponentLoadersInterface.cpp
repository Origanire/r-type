#include "EngineComponentLoaders.hpp"

#include <cstdint>
#include <functional>
#include <utility>

#include "Bounce.hpp"
#include "Bounded.hpp"
#include "BoundText.hpp"
#include "Button.hpp"
#include "Choice.hpp"
#include "Collider.hpp"
#include "ContactDamage.hpp"
#include "DataReaders.hpp"
#include "DeathEffect.hpp"
#include "DespawnOffscreen.hpp"
#include "FadeWhen.hpp"
#include "Focusable.hpp"
#include "Foreground.hpp"
#include "Gravity.hpp"
#include "Health.hpp"
#include "Hidden.hpp"
#include "Invulnerable.hpp"
#include "Lifetime.hpp"
#include "ListView.hpp"
#include "Lives.hpp"
#include "MusicWhen.hpp"
#include "Parallax.hpp"
#include "ParticleEmitter.hpp"
#include "Persistent.hpp"
#include "PlayerControl.hpp"
#include "PlayerLabel.hpp"
#include "ProgressBar.hpp"
#include "Pushable.hpp"
#include "RectShape.hpp"
#include "Replicated.hpp"
#include "Respawn.hpp"
#include "Rotation.hpp"
#include "SceneError.hpp"
#include "Score.hpp"
#include "ScoreValue.hpp"
#include "Script.hpp"
#include "Solid.hpp"
#include "SoundCue.hpp"
#include "Sprite.hpp"
#include "SpriteAnimation.hpp"
#include "StatusText.hpp"
#include "Text.hpp"
#include "TextField.hpp"
#include "Tilemap.hpp"
#include "TiltAnimation.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "VisibleWhen.hpp"
#include "Weapon.hpp"

namespace engine::scene {

void EngineComponentLoaders::loadButton(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    ecs::Button &button = registry.emplace<ecs::Button>(
        entity, data.getString("action"), data.getFloat("width"), data.getFloat("height"));
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
    registry.emplace<ecs::ListView>(entity, data.getString("source"), data.getFloat("spacing"),
        static_cast<std::size_t>(maxRows), std::move(parts));
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
    registry.emplace<ecs::ProgressBar>(entity, data.getString("key"), data.getFloat("width"), data.getFloat("max", 1.f),
        data.getBool("vertical", false));
}

void EngineComponentLoaders::loadFadeWhen(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::FadeWhen>(
        entity, data.getString("key"), data.getString("value"), data.getFloat("seconds", 0.f));
}

void EngineComponentLoaders::loadChoice(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    std::vector<std::string> options;
    for (const DataNode &option : data.child("options").items())
        options.push_back(option.asString());
    registry.emplace<ecs::Choice>(entity, data.getString("name"), std::move(options));
}

} // namespace engine::scene
