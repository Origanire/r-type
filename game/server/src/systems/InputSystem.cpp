#include "InputSystem.hpp"

#include <cstdint>

#include "GameConstants.hpp"
#include "RtypeProtocol.hpp"

namespace rtype::server {

namespace {

float axis(std::uint8_t buttons, std::uint8_t positive, std::uint8_t negative)
{
    float value = 0.f;
    if (buttons & positive)
        value += 1.f;
    if (buttons & negative)
        value -= 1.f;
    return value;
}

} // namespace

InputSystem::InputSystem(ServerWorld &world) : m_world(world)
{
}

void InputSystem::update(float)
{
    for (std::size_t i : m_world.entities.with(PLAYER_INPUT | VELOCITY)) {
        const std::uint8_t buttons = m_world.inputs[i].buttons;
        m_world.velocities[i].x = axis(buttons, protocol::BUTTON_RIGHT, protocol::BUTTON_LEFT) * PLAYER_SPEED;
        m_world.velocities[i].y = axis(buttons, protocol::BUTTON_DOWN, protocol::BUTTON_UP) * PLAYER_SPEED;
    }
}

} // namespace rtype::server
