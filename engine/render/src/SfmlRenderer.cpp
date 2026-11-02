#include "SfmlRenderer.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

namespace engine::render {

namespace {

sf::Color toSfml(Color color)
{
    return sf::Color(color.r(), color.g(), color.b(), color.a());
}

} // namespace

SfmlRenderer::SfmlRenderer(Window &window, const TextureManager &textures, const FontManager &fonts)
    : m_window(window), m_textures(textures), m_fonts(fonts), m_textScale(1.f)
{
}

void SfmlRenderer::setTextScale(float scale)
{
    if (scale > 0.f)
        m_textScale = scale;
}

void SfmlRenderer::clear(Color color)
{
    m_window.native().clear(toSfml(color));
}

void SfmlRenderer::drawRect(Vector2 position, Vector2 size, Color color)
{
    sf::RectangleShape rect({size.x(), size.y()});
    rect.setPosition({position.x(), position.y()});
    rect.setFillColor(toSfml(color));
    m_window.native().draw(rect);
}

void SfmlRenderer::drawTexture(const std::string &texture, IntRect source, Vector2 position, Vector2 scale)
{
    sf::Sprite sprite(m_textures.get(texture));
    sprite.setTextureRect(sf::IntRect({source.x(), source.y()}, {source.width(), source.height()}));
    sprite.setPosition({position.x(), position.y()});
    sprite.setScale({scale.x(), scale.y()});
    m_window.native().draw(sprite);
}

void SfmlRenderer::drawText(
    const std::string &font, const std::string &text, unsigned int size, Vector2 position, Color color)
{
    sf::Text label(m_fonts.get(font), text, static_cast<unsigned int>(static_cast<float>(size) * m_textScale));
    label.setPosition({position.x(), position.y()});
    label.setFillColor(toSfml(color));
    m_window.native().draw(label);
}

void SfmlRenderer::display()
{
    m_window.native().display();
}

} // namespace engine::render
