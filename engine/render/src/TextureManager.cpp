#include "TextureManager.hpp"

#include <utility>

#include "RenderError.hpp"

namespace engine::render {

void TextureManager::load(const std::string &name, const std::string &path)
{
    sf::Texture texture;
    if (!texture.loadFromFile(path))
        throw RenderError("TextureManager: cannot load " + path);
    m_textures.insert_or_assign(name, std::move(texture));
}

const sf::Texture &TextureManager::get(const std::string &name) const
{
    auto it = m_textures.find(name);
    if (it == m_textures.end())
        throw RenderError("TextureManager: unknown texture " + name);
    return it->second;
}

} // namespace engine::render
