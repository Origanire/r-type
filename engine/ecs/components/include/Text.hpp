#pragma once

#include <string>

#include "Color.hpp"
#include "IComponent.hpp"

namespace engine::ecs {

class Text : public IComponent
{
public:
    Text(std::string font, std::string content, unsigned int size, Color color);

    const std::string &font() const;
    const std::string &content() const;
    void setContent(std::string content);
    unsigned int size() const;
    Color color() const;
    void setColor(Color color);

private:
    std::string m_font;
    std::string m_content;
    unsigned int m_size;
    Color m_color;
};

} // namespace engine::ecs
