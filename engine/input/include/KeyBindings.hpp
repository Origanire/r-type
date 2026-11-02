#pragma once

#include <array>
#include <cstddef>
#include <string>

#include <SFML/Window/Keyboard.hpp>

#include "Action.hpp"

namespace engine::input {

class KeyBindings
{
public:
    static constexpr std::size_t ACTION_COUNT = static_cast<std::size_t>(Action::Fire) + 1;

    KeyBindings();

    void bind(Action action, sf::Keyboard::Key key);
    sf::Keyboard::Key keyFor(Action action) const;
    static sf::Keyboard::Key keyFromName(const std::string &name);

private:
    std::array<sf::Keyboard::Key, ACTION_COUNT> m_keys;
};

} // namespace engine::input
