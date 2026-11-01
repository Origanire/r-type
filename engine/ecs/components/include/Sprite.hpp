#pragma once

#include <string>

#include "IComponent.hpp"
#include "RenderTypes.hpp"

namespace engine::ecs {

class Sprite : public IComponent
{
public:
    Sprite(std::string texture, render::IntRect source, float scale);

    const std::string &texture() const;
    render::IntRect source() const;
    void setSource(render::IntRect source);
    float scale() const;

private:
    std::string m_texture;
    render::IntRect m_source;
    float m_scale;
};

} // namespace engine::ecs
