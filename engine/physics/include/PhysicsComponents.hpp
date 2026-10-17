#pragma once

namespace engine::physics {

// Top-left corner of the entity, in world units.
struct Position
{
    float x = 0.f;
    float y = 0.f;
};

// World units per second.
struct Velocity
{
    float x = 0.f;
    float y = 0.f;
};

// Size of the collision box, starting at the entity position.
struct Hitbox
{
    float width = 0.f;
    float height = 0.f;
};

} // namespace engine::physics
