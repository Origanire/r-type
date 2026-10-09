#include "SfmlRenderer.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace engine::render {

namespace {

sf::Color toSfml(Color color)
{
    return sf::Color(color.r, color.g, color.b, color.a);
}

} // namespace

SfmlRenderer::SfmlRenderer(Window &window, const TextureManager &textures) : m_window(window), m_textures(textures)
{
}

void SfmlRenderer::clear(Color color)
{
    m_window.native().clear(toSfml(color));
}

void SfmlRenderer::drawRect(Vector2 position, Vector2 size, Color color)
{
    sf::RectangleShape rect({size.x, size.y});
    rect.setPosition({position.x, position.y});
    rect.setFillColor(toSfml(color));
    m_window.native().draw(rect);
}

void SfmlRenderer::drawTexture(const std::string &texture, IntRect source, Vector2 position, Vector2 scale)
{
    sf::Sprite sprite(m_textures.get(texture));
    sprite.setTextureRect(sf::IntRect({source.x, source.y}, {source.width, source.height}));
    sprite.setPosition({position.x, position.y});
    sprite.setScale({scale.x, scale.y});
    m_window.native().draw(sprite);
}

void SfmlRenderer::display()
{
    m_window.native().display();
}

} // namespace engine::render
