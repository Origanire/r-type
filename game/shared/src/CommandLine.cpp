#include "CommandLine.hpp"

#include "GameError.hpp"
#include "PortOption.hpp"
#include "RtypeProtocol.hpp"

namespace rtype {

namespace {

constexpr const char *DEFAULT_HOST = "127.0.0.1";
constexpr const char *DEFAULT_GAME = "rtype";

} // namespace

CommandLine::CommandLine(int argc, char **argv, std::string_view usage, bool acceptsHost)
    : m_host(DEFAULT_HOST), m_port(protocol::DEFAULT_PORT), m_game(DEFAULT_GAME)
{
    for (int i = 1; i < argc; ++i) {
        const std::string_view option(argv[i]);
        const bool known = option == "-p" || option == "-g" || (acceptsHost && option == "-h");
        if (!known || i + 1 >= argc)
            throw GameError(std::string(usage));
        const std::string value = argv[++i];
        if (option == "-p")
            m_port = parsePort(value);
        else if (option == "-g")
            m_game = value;
        else
            m_host = value;
    }
    if (m_game.empty() || m_game.find_first_of("/\\.") != std::string::npos)
        throw GameError("CommandLine: invalid game name " + m_game);
}

const std::string &CommandLine::host() const
{
    return m_host;
}

std::uint16_t CommandLine::port() const
{
    return m_port;
}

const std::string &CommandLine::game() const
{
    return m_game;
}

} // namespace rtype
