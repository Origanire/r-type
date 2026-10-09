#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include "Entity.hpp"

namespace engine::ecs {

class Registry;

class CommandBuffer
{
public:
    using SpawnSetup = std::function<void(Registry &, const Entity &)>;

    void spawn(SpawnSetup setup = nullptr);
    void kill(const Entity &entity);
    void apply(Registry &registry);
    std::size_t size() const;

private:
    std::vector<std::function<void(Registry &)>> m_commands;
};

} // namespace engine::ecs
