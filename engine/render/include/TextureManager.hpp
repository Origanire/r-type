#pragma once

#include <string>
#include <unordered_map>

#include <SFML/Graphics/Texture.hpp>

namespace engine::render {

class TextureManager
{
public:
    void load(const std::string &name, const std::string &path);
    const sf::Texture &get(const std::string &name) const;

private:
    std::unordered_map<std::string, sf::Texture> m_textures;
};

} // namespace engine::render
