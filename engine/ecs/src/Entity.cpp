#include "Entity.hpp"

namespace engine::ecs {

Entity::Entity(std::size_t index, Generation generation) noexcept
    : m_index(index), m_generation(generation)
{
}

Entity::operator std::size_t() const noexcept
{
    return m_index;
}

std::size_t Entity::index() const noexcept
{
    return m_index;
}

Entity::Generation Entity::generation() const noexcept
{
    return m_generation;
}

} // namespace engine::ecs
