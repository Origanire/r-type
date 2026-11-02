#include "Hud.hpp"

#include <cstdint>

#include "GameError.hpp"
#include "Text.hpp"
#include "Transform.hpp"

namespace rtype::client {

namespace {

constexpr std::size_t COLOR_VALUES = 4;

engine::Color readColor(const engine::scene::DataNode &values)
{
    const std::vector<engine::scene::DataNode> channels = values.items();
    if (channels.size() != COLOR_VALUES)
        throw GameError("Hud: " + values.path() + " must be [r, g, b, a]");
    return {static_cast<std::uint8_t>(channels[0].asInt()), static_cast<std::uint8_t>(channels[1].asInt()),
        static_cast<std::uint8_t>(channels[2].asInt()), static_cast<std::uint8_t>(channels[3].asInt())};
}

std::vector<engine::Color> readColors(const engine::scene::DataNode &list)
{
    std::vector<engine::Color> colors;
    for (const engine::scene::DataNode &color : list.items())
        colors.push_back(readColor(color));
    if (colors.size() < protocol::MAX_PLAYERS)
        throw GameError("Hud: one color per player is required in " + list.path());
    return colors;
}

unsigned int scaled(int size, float scale)
{
    return static_cast<unsigned int>(static_cast<float>(size) * scale);
}

} // namespace

Hud::Hud(engine::ecs::Registry &registry, const engine::scene::DataNode &hud, float textScale)
    : m_registry(registry), m_font(hud.getString("font")), m_gameOverText(hud.getString("gameOverText")),
      m_colors(readColors(hud.child("playerColors"))), m_labelSize(scaled(hud.getInt("labelSize"), textScale)),
      m_labelOffsetX(hud.getFloat("labelOffsetX")), m_labelOffsetY(hud.getFloat("labelOffsetY") * textScale),
      m_gameOver(createText(hud.getFloat("gameOverX"), hud.getFloat("gameOverY"),
          scaled(hud.getInt("gameOverSize"), textScale), engine::Color()))
{
    const unsigned int size = scaled(hud.getInt("size"), textScale);
    for (std::size_t slot = 0; slot < protocol::MAX_PLAYERS; ++slot) {
        const float x = hud.getFloat("x") + static_cast<float>(slot) * hud.getFloat("spacing");
        m_lines.push_back(createText(x, hud.getFloat("y"), size, m_colors[slot]));
    }
}

void Hud::update(const protocol::WorldSnapshot &snapshot)
{
    for (std::size_t slot = 0; slot < m_lines.size(); ++slot) {
        const protocol::PlayerState &player = snapshot.players[slot];
        std::string content;
        if (player.state != protocol::SLOT_EMPTY) {
            content = "P" + std::to_string(slot + 1) + "  LIVES " + std::to_string(player.lives) + "  SCORE "
                + std::to_string(player.score);
        }
        m_registry.get<engine::ecs::Text>(m_lines[slot]).setContent(content);
    }
    const bool over = snapshot.gameState == protocol::GAME_OVER;
    m_registry.get<engine::ecs::Text>(m_gameOver).setContent(over ? m_gameOverText : std::string());
}

engine::Color Hud::playerColor(std::size_t slot) const
{
    return m_colors[slot % m_colors.size()];
}

const std::string &Hud::font() const
{
    return m_font;
}

unsigned int Hud::labelSize() const
{
    return m_labelSize;
}

float Hud::labelOffsetX() const
{
    return m_labelOffsetX;
}

float Hud::labelOffsetY() const
{
    return m_labelOffsetY;
}

engine::ecs::Entity Hud::createText(float x, float y, unsigned int size, engine::Color color)
{
    const engine::ecs::Entity entity = m_registry.spawn();
    m_registry.emplace<engine::ecs::Transform>(entity, x, y);
    m_registry.emplace<engine::ecs::Text>(entity, m_font, std::string(), size, color);
    return entity;
}

} // namespace rtype::client
