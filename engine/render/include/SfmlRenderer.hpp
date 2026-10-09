#pragma once

#include <string>

#include "IRenderer.hpp"
#include "TextureManager.hpp"
#include "Window.hpp"

namespace engine::render {

class SfmlRenderer : public IRenderer
{
public:
    SfmlRenderer(Window &window, const TextureManager &textures);

    void clear(Color color) override;
    void drawRect(Vector2 position, Vector2 size, Color color) override;
    void drawTexture(const std::string &texture, IntRect source, Vector2 position, Vector2 scale) override;
    void display() override;

private:
    Window &m_window;
    const TextureManager &m_textures;
};

} // namespace engine::render
