#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

#include "AssetPath.hpp"
#include "Client.hpp"
#include "EngineError.hpp"
#include "GameError.hpp"
#include "PortOption.hpp"
#include "RtypeProtocol.hpp"
#include "UdpTransport.hpp"

namespace {

constexpr std::string_view USAGE = "Usage: r-type_client [-h IPv4-address] [-p port]";
constexpr const char *DEFAULT_HOST = "127.0.0.1";

engine::net::Endpoint parseArguments(int argc, char **argv)
{
    std::string host = DEFAULT_HOST;
    std::uint16_t port = rtype::protocol::DEFAULT_PORT;
    for (int i = 1; i < argc; ++i) {
        const std::string_view option(argv[i]);
        if ((option != "-h" && option != "-p") || i + 1 >= argc)
            throw rtype::GameError(std::string(USAGE));
        if (option == "-h")
            host = argv[++i];
        else
            port = rtype::parsePort(argv[++i]);
    }
    return engine::net::UdpTransport::numericEndpoint(host, port);
}

} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        std::cout << USAGE << std::endl;
        return 0;
    }
    try {
        rtype::client::Client client(rtype::findAssets(argv[0]), rtype::findData(argv[0]), parseArguments(argc, argv));
        client.run();
    } catch (const engine::EngineError &error) {
        std::cerr << "r-type_client: " << error.what() << std::endl;
        return 84;
    } catch (const std::exception &error) {
        std::cerr << "r-type_client: unexpected error: " << error.what() << std::endl;
        return 84;
    }
    return 0;
}
