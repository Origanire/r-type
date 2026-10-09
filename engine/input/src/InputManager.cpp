#include "InputManager.hpp"

#include <cstddef>
#include <utility>

namespace engine::input {

InputManager::InputManager(KeyBindings bindings) : m_bindings(std::move(bindings))
{
}

void InputManager::update(bool windowFocused)
{
    for (std::size_t i = 0; i < ACTION_COUNT; ++i) {
        Action action = static_cast<Action>(i);
        m_pressed[i] = windowFocused && sf::Keyboard::isKeyPressed(m_bindings.keyFor(action));
    }
}

bool InputManager::isPressed(Action action) const
{
    return m_pressed[static_cast<std::size_t>(action)];
}

std::uint8_t InputManager::toBits() const
{
    std::uint8_t bits = 0;
    for (std::size_t i = 0; i < ACTION_COUNT; ++i) {
        if (m_pressed[i])
            bits = static_cast<std::uint8_t>(bits | (1u << i));
    }
    return bits;
}

KeyBindings &InputManager::bindings()
{
    return m_bindings;
}

} // namespace engine::input
