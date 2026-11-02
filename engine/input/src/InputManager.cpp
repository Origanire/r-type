#include "InputManager.hpp"

#include <cstddef>
#include <utility>

namespace engine::input {

InputManager::InputManager(KeyBindings bindings) : m_bindings(std::move(bindings))
{
}

void InputManager::update(bool windowFocused)
{
    for (std::size_t i = 0; i < KeyBindings::ACTION_COUNT; ++i) {
        Action action = static_cast<Action>(i);
        const bool down = windowFocused && sf::Keyboard::isKeyPressed(m_bindings.keyFor(action));
        if (m_toggle[i] && down && !m_wasDown[i])
            m_latched[i] = !m_latched[i];
        m_pressed[i] = m_toggle[i] ? m_latched[i] : down;
        m_wasDown[i] = down;
    }
}

bool InputManager::isPressed(Action action) const
{
    return m_pressed[static_cast<std::size_t>(action)];
}

std::uint8_t InputManager::toBits() const
{
    std::uint8_t bits = 0;
    for (std::size_t i = 0; i < KeyBindings::ACTION_COUNT; ++i) {
        if (m_pressed[i])
            bits = static_cast<std::uint8_t>(bits | (1u << i));
    }
    return bits;
}

KeyBindings &InputManager::bindings()
{
    return m_bindings;
}

void InputManager::setToggle(Action action, bool toggle)
{
    const std::size_t index = static_cast<std::size_t>(action);
    m_toggle[index] = toggle;
    m_latched[index] = false;
}

} // namespace engine::input
