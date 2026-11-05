#include "VisibleWhen.hpp"

#include <utility>

namespace engine::ecs {

VisibleWhen::VisibleWhen(std::string key, std::string value) : m_key(std::move(key)), m_value(std::move(value))
{
}

const std::string &VisibleWhen::key() const
{
    return m_key;
}

const std::string &VisibleWhen::value() const
{
    return m_value;
}

} // namespace engine::ecs
