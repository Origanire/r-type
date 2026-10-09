#pragma once

namespace engine::ecs {

class Registry;
class CommandBuffer;

struct SystemContext
{
    Registry &registry;
    CommandBuffer &commands;
    float dt;
};

} // namespace engine::ecs
