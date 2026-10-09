#include "PlayerControlSystem.hpp"

#include "Zipper.hpp"

namespace rtype::client {

using engine::input::Action;

PlayerControlSystem::PlayerControlSystem(const engine::input::InputManager &input) : m_input(input)
{
}

void PlayerControlSystem::operator()(engine::ecs::SystemContext &, engine::ecs::SparseArray<PlayerControl> &controls,
    engine::ecs::SparseArray<Velocity> &velocities) const
{
    float dx = (m_input.isPressed(Action::Right) ? 1.f : 0.f) - (m_input.isPressed(Action::Left) ? 1.f : 0.f);
    float dy = (m_input.isPressed(Action::Down) ? 1.f : 0.f) - (m_input.isPressed(Action::Up) ? 1.f : 0.f);

    for (auto [control, velocity] : engine::ecs::Zipper(controls, velocities)) {
        velocity.x = dx * control.speed;
        velocity.y = dy * control.speed;
    }
}

} // namespace rtype::client
