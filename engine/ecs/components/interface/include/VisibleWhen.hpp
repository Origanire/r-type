#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class VisibleWhen : public IComponent
{
public:
    VisibleWhen(std::string key, std::string value);

    const std::string &key() const;
    const std::string &value() const;

private:
    std::string m_key;
    std::string m_value;
};

} // namespace engine::ecs
