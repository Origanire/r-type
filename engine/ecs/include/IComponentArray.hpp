#pragma once

#include <cstddef>

namespace engine::ecs {

class IComponentArray
{
public:
    virtual ~IComponentArray() = default;

    virtual void erase(std::size_t index) = 0;
};

} // namespace engine::ecs
