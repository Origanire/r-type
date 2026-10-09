#pragma once

#include <cstddef>
#include <cstdint>

namespace engine::ecs {

class Registry;

class Entity
{
public:
    using Generation = std::uint32_t;

    operator std::size_t() const noexcept;

    std::size_t index() const noexcept;
    Generation generation() const noexcept;

    bool operator==(const Entity &other) const noexcept = default;

private:
    explicit Entity(std::size_t index, Generation generation = 0) noexcept;

    friend class Registry;

    std::size_t m_index;
    Generation m_generation;
};

} // namespace engine::ecs
