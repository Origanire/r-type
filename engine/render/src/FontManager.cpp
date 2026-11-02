#include "FontManager.hpp"

#include <utility>

#include "RenderError.hpp"

namespace engine::render {

void FontManager::load(const std::string &name, const std::string &path)
{
    sf::Font font;
    if (!font.openFromFile(path))
        throw RenderError("FontManager: cannot load " + path);
    m_fonts.insert_or_assign(name, std::move(font));
}

const sf::Font &FontManager::get(const std::string &name) const
{
    const auto found = m_fonts.find(name);
    if (found == m_fonts.end())
        throw RenderError("FontManager: unknown font " + name);
    return found->second;
}

} // namespace engine::render
