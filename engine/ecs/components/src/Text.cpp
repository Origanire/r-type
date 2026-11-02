#include "Text.hpp"

#include <utility>

namespace engine::ecs {

Text::Text(std::string font, std::string content, unsigned int size, Color color)
    : m_font(std::move(font)), m_content(std::move(content)), m_size(size), m_color(color)
{
}

const std::string &Text::font() const
{
    return m_font;
}

const std::string &Text::content() const
{
    return m_content;
}

void Text::setContent(std::string content)
{
    m_content = std::move(content);
}

unsigned int Text::size() const
{
    return m_size;
}

Color Text::color() const
{
    return m_color;
}

void Text::setColor(Color color)
{
    m_color = color;
}

} // namespace engine::ecs
