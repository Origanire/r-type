#pragma once

#include <array>
#include <cstdint>

#include "Action.hpp"
#include "KeyBindings.hpp"

namespace engine::input {

class InputManager
{
public:
    explicit InputManager(KeyBindings bindings = KeyBindings());

    void update(bool windowFocused);
    bool isPressed(Action action) const;
    std::uint8_t toBits() const;

    KeyBindings &bindings();

private:
    KeyBindings m_bindings;
    std::array<bool, ACTION_COUNT> m_pressed{};
};

} // namespace engine::input
