#pragma once

#include <array>

#include <SFML/Window/Keyboard.hpp>

#include "Action.hpp"

namespace engine::input {

class KeyBindings
{
public:
    KeyBindings();

    void bind(Action action, sf::Keyboard::Key key);
    sf::Keyboard::Key keyFor(Action action) const;

private:
    std::array<sf::Keyboard::Key, ACTION_COUNT> m_keys;
};

} // namespace engine::input
