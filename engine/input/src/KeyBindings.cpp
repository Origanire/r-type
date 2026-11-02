#include "KeyBindings.hpp"

#include <array>
#include <cctype>
#include <cstddef>
#include <utility>

#include "InputError.hpp"

namespace engine::input {

namespace {

const std::array<std::pair<const char *, sf::Keyboard::Key>, 14> NAMED_KEYS = {{
    {"Up", sf::Keyboard::Key::Up},
    {"Down", sf::Keyboard::Key::Down},
    {"Left", sf::Keyboard::Key::Left},
    {"Right", sf::Keyboard::Key::Right},
    {"Space", sf::Keyboard::Key::Space},
    {"Enter", sf::Keyboard::Key::Enter},
    {"Tab", sf::Keyboard::Key::Tab},
    {"Backspace", sf::Keyboard::Key::Backspace},
    {"LShift", sf::Keyboard::Key::LShift},
    {"RShift", sf::Keyboard::Key::RShift},
    {"LControl", sf::Keyboard::Key::LControl},
    {"RControl", sf::Keyboard::Key::RControl},
    {"LAlt", sf::Keyboard::Key::LAlt},
    {"RAlt", sf::Keyboard::Key::RAlt},
}};

} // namespace

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

sf::Keyboard::Key KeyBindings::keyFromName(const std::string &name)
{
    if (name.size() == 1 && std::isalpha(static_cast<unsigned char>(name[0]))) {
        const int offset = std::toupper(static_cast<unsigned char>(name[0])) - 'A';
        return static_cast<sf::Keyboard::Key>(static_cast<int>(sf::Keyboard::Key::A) + offset);
    }
    if (name.size() == 1 && std::isdigit(static_cast<unsigned char>(name[0]))) {
        const int offset = name[0] - '0';
        return static_cast<sf::Keyboard::Key>(static_cast<int>(sf::Keyboard::Key::Num0) + offset);
    }
    for (const auto &[keyName, key] : NAMED_KEYS) {
        if (name == keyName)
            return key;
    }
    throw InputError("KeyBindings: unknown key name " + name);
}

} // namespace engine::input
