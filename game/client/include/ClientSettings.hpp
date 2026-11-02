#pragma once

#include <string>

#include "DataNode.hpp"
#include "InputManager.hpp"

namespace rtype::client {

class ClientSettings
{
public:
    explicit ClientSettings(const engine::scene::DataNode &settings);

    void applyControls(engine::input::InputManager &input) const;
    bool showPlayerLabels() const;
    float musicVolume() const;
    float soundVolume() const;
    float textScale() const;

private:
    std::string m_up;
    std::string m_down;
    std::string m_left;
    std::string m_right;
    std::string m_fire;
    bool m_toggleFire;
    bool m_showPlayerLabels;
    float m_musicVolume;
    float m_soundVolume;
    float m_textScale;
};

} // namespace rtype::client
