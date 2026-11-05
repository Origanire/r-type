#pragma once

#include <array>
#include <cstddef>
#include <string>

#include "UiKey.hpp"
#include "Vector2.hpp"

namespace engine {

class UiInput
{
public:
    static constexpr std::size_t KEY_COUNT = static_cast<std::size_t>(UiKey::Console) + 1;
    static constexpr char FIRST_PRINTABLE = ' ';
    static constexpr char LAST_PRINTABLE = '~';

    UiInput();

    void beginFrame();
    void movePointer(Vector2 position);
    void pressPointer(Vector2 position);
    void releasePointer();
    void typeCharacter(char character);
    void pressKey(UiKey key);

    Vector2 pointer() const;
    bool pointerMoved() const;
    bool clicked() const;
    bool pointerDown() const;
    const std::string &typedText() const;
    bool keyPressed(UiKey key) const;

private:
    Vector2 m_pointer;
    bool m_pointerMoved;
    bool m_clicked;
    bool m_pointerDown;
    std::string m_typedText;
    std::array<bool, KEY_COUNT> m_keys;
};

} // namespace engine
