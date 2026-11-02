#include <exception>
#include <iostream>
#include <string_view>

#include "AssetPath.hpp"
#include "Client.hpp"
#include "CommandLine.hpp"
#include "EngineError.hpp"
#include "UdpTransport.hpp"

namespace {

constexpr std::string_view USAGE = "Usage: r-type_client [-h IPv4-address] [-p port] [-g game]";

} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        std::cout << USAGE << std::endl;
        return 0;
    }
    try {
        const rtype::CommandLine options(argc, argv, USAGE, true);
        rtype::client::Client client(rtype::findAssets(argv[0]), rtype::findData(argv[0]), options.game(),
            engine::net::UdpTransport::numericEndpoint(options.host(), options.port()));
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
