#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace rtype {

class CommandLine
{
public:
    CommandLine(int argc, char **argv, std::string_view usage, bool acceptsHost);

    const std::string &host() const;
    std::uint16_t port() const;
    const std::string &game() const;

private:
    std::string m_host;
    std::uint16_t m_port;
    std::string m_game;
};

} // namespace rtype
