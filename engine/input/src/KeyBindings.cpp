#include "KeyBindings.hpp"

#include <cstddef>

namespace engine::input {

KeyBindings::KeyBindings()
    : m_keys{sf::Keyboard::Key::Up, sf::Keyboard::Key::Down, sf::Keyboard::Key::Left, sf::Keyboard::Key::Right,
          sf::Keyboard::Key::Space}
{
}

void KeyBindings::bind(Action action, sf::Keyboard::Key key)
{
    m_keys[static_cast<std::size_t>(action)] = key;
}

sf::Keyboard::Key KeyBindings::keyFor(Action action) const
{
    return m_keys[static_cast<std::size_t>(action)];
}

} // namespace engine::input
