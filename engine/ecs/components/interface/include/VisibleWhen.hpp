#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class VisibleWhen : public IComponent
{
public:
    VisibleWhen(std::string key, std::string value, std::string unlessKey = {}, std::string unlessValue = {});

    const std::string &key() const;
    const std::string &value() const;
    const std::string &unlessKey() const;
    const std::string &unlessValue() const;

private:
    std::string m_key;
    std::string m_value;
    std::string m_unlessKey;
    std::string m_unlessValue;
};

} // namespace engine::ecs
