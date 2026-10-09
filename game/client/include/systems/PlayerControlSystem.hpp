#pragma once

#include "InputManager.hpp"
#include "PlayerControl.hpp"
#include "SparseArray.hpp"
#include "SystemContext.hpp"
#include "Velocity.hpp"

namespace rtype::client {

class PlayerControlSystem
{
public:
    explicit PlayerControlSystem(const engine::input::InputManager &input);

    void operator()(engine::ecs::SystemContext &context, engine::ecs::SparseArray<PlayerControl> &controls,
        engine::ecs::SparseArray<Velocity> &velocities) const;

private:
    const engine::input::InputManager &m_input;
};

} // namespace rtype::client
