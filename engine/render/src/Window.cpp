#include "Window.hpp"

#include <optional>

#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>

#include "RenderError.hpp"

namespace engine::render {

Window::Window(unsigned int width, unsigned int height, const std::string &title, Vector2 worldSize)
    : m_window(sf::VideoMode({width, height}), title)
{
    if (!m_window.isOpen())
        throw RenderError("Window: cannot open the window");
    m_window.setFramerateLimit(60);
    m_window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {worldSize.x, worldSize.y})));
}

void Window::processEvents()
{
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
        } else if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape)
                m_window.close();
        }
    }
}

bool Window::isOpen() const
{
    return m_window.isOpen();
}

bool Window::hasFocus() const
{
    return m_window.hasFocus();
}

void Window::close()
{
    m_window.close();
}

sf::RenderWindow &Window::native()
{
    return m_window;
}

} // namespace engine::render
