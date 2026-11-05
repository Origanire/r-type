#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>

#include <SFML/Window/Keyboard.hpp>

#include "Action.hpp"

namespace engine::input {

class KeyBindings
{
public:
    static constexpr std::size_t ACTION_COUNT = static_cast<std::size_t>(Action::Special) + 1;
    static constexpr float DEFAULT_DEADZONE = 35.f;
    static constexpr std::size_t NAMED_KEY_COUNT = 14;

    KeyBindings();

    void bind(Action action, sf::Keyboard::Scancode key);
    sf::Keyboard::Scancode keyFor(Action action) const;
    void bindButton(Action action, unsigned int button);
    std::optional<unsigned int> buttonFor(Action action) const;
    void setDeadzone(float deadzone);
    float deadzone() const;

    static sf::Keyboard::Scancode keyFromName(const std::string &name);

private:
    std::array<sf::Keyboard::Scancode, ACTION_COUNT> m_keys;
    std::array<std::optional<unsigned int>, ACTION_COUNT> m_buttons;
    float m_deadzone;
};

} // namespace engine::input
