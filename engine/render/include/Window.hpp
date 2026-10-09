#pragma once

#include <string>

#include <SFML/Graphics/RenderWindow.hpp>

#include "RenderTypes.hpp"

namespace engine::render {

class Window
{
public:
    Window(unsigned int width, unsigned int height, const std::string &title, Vector2 worldSize);

    void processEvents();
    bool isOpen() const;
    bool hasFocus() const;
    void close();

    sf::RenderWindow &native();

private:
    sf::RenderWindow m_window;
};

} // namespace engine::render
