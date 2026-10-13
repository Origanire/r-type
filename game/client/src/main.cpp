#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <stdexcept>

#include "Client.hpp"
#include "EngineError.hpp"
#include "PortOption.hpp"

namespace {

std::filesystem::path findAssets(const char *programPath)
{
    std::filesystem::path besideProgram = std::filesystem::absolute(programPath).parent_path() / "assets";
    if (std::filesystem::exists(besideProgram))
        return besideProgram;
    return std::filesystem::current_path() / "assets";
}

} // namespace

int main(int argc, char **argv)
{
    try {
        std::string host = "127.0.0.1";
        std::uint16_t port = rtype::protocol::DEFAULT_PORT;
        for (int i = 1; i < argc; ++i) {
            const std::string_view option(argv[i]);
            if (option == "--help") {
                std::cout << "Usage: r-type_client [-h IPv4-address] [-p port]\n";
                return 0;
            }
            if ((option != "-h" && option != "-p") || ++i >= argc)
                throw std::runtime_error("Usage: r-type_client [-h IPv4-address] [-p port]");
            if (option == "-h")
                host = argv[i];
            else
                port = rtype::parsePort(argv[i]);
        }
        rtype::client::Client client(findAssets(argv[0]), engine::net::UdpTransport::numericEndpoint(host, port));
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
