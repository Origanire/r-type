#include "InputSystem.hpp"

#include <cstdint>

#include "GameConstants.hpp"
#include "PlayerInput.hpp"
#include "Registry.hpp"
#include "RtypeProtocol.hpp"
#include "Velocity.hpp"

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

void inputSystem(engine::ecs::SystemContext &context)
{
    auto &inputs = context.registry.getComponents<PlayerInput>();
    auto &velocities = context.registry.getComponents<Velocity>();
    for (std::size_t i = 0; i < inputs.size() && i < velocities.size(); ++i) {
        if (!inputs[i] || !velocities[i])
            continue;
        const std::uint8_t buttons = inputs[i]->buttons;
        velocities[i]->x = axis(buttons, protocol::BUTTON_RIGHT, protocol::BUTTON_LEFT) * PLAYER_SPEED;
        velocities[i]->y = axis(buttons, protocol::BUTTON_DOWN, protocol::BUTTON_UP) * PLAYER_SPEED;
    }
}

} // namespace rtype::server
