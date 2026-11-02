#include "PlayerSettings.hpp"

#include "KeyBindings.hpp"

namespace engine::client {

namespace {

constexpr float DEFAULT_MUSIC_VOLUME = 40.f;
constexpr float DEFAULT_SOUND_VOLUME = 70.f;

} // namespace

PlayerSettings::PlayerSettings(const scene::DataNode &settings)
    : m_up(settings.child("controls").getString("up")), m_down(settings.child("controls").getString("down")),
      m_left(settings.child("controls").getString("left")), m_right(settings.child("controls").getString("right")),
      m_fire(settings.child("controls").getString("fire")), m_toggleFire(settings.getBool("toggleFire", false)),
      m_showPlayerLabels(settings.getBool("showPlayerLabels", true)),
      m_musicVolume(settings.getFloat("musicVolume", DEFAULT_MUSIC_VOLUME)),
      m_soundVolume(settings.getFloat("soundVolume", DEFAULT_SOUND_VOLUME)),
      m_textScale(settings.getFloat("textScale", 1.f))
{
}

void PlayerSettings::applyControls(input::InputManager &input) const
{
    using input::Action;
    using input::KeyBindings;

    input.bindings().bind(Action::Up, KeyBindings::keyFromName(m_up));
    input.bindings().bind(Action::Down, KeyBindings::keyFromName(m_down));
    input.bindings().bind(Action::Left, KeyBindings::keyFromName(m_left));
    input.bindings().bind(Action::Right, KeyBindings::keyFromName(m_right));
    input.bindings().bind(Action::Fire, KeyBindings::keyFromName(m_fire));
    input.setToggle(Action::Fire, m_toggleFire);
}

bool PlayerSettings::showPlayerLabels() const
{
    return m_showPlayerLabels;
}

float PlayerSettings::musicVolume() const
{
    return m_musicVolume;
}

float PlayerSettings::soundVolume() const
{
    return m_soundVolume;
}

float PlayerSettings::textScale() const
{
    return m_textScale;
}

} // namespace engine::client
