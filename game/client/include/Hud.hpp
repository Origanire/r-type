#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Color.hpp"
#include "DataNode.hpp"
#include "Entity.hpp"
#include "Registry.hpp"
#include "RtypeProtocol.hpp"

namespace rtype::client {

class Hud
{
public:
    Hud(engine::ecs::Registry &registry, const engine::scene::DataNode &hud, float textScale);

    void update(const protocol::WorldSnapshot &snapshot);
    engine::Color playerColor(std::size_t slot) const;
    const std::string &font() const;
    unsigned int labelSize() const;
    float labelOffsetX() const;
    float labelOffsetY() const;

private:
    engine::ecs::Entity createText(float x, float y, unsigned int size, engine::Color color);

    engine::ecs::Registry &m_registry;
    std::string m_font;
    std::string m_gameOverText;
    std::vector<engine::Color> m_colors;
    unsigned int m_labelSize;
    float m_labelOffsetX;
    float m_labelOffsetY;
    std::vector<engine::ecs::Entity> m_lines;
    engine::ecs::Entity m_gameOver;
};

} // namespace rtype::client
