#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>

#include "AssetPath.hpp"
#include "CommandLine.hpp"
#include "DataNode.hpp"
#include "EngineError.hpp"
#include "NetworkSettings.hpp"
#include "OnlineClient.hpp"
#include "UdpTransport.hpp"

int main(int argc, char **argv)
{
    constexpr std::string_view usage = "Usage: r-type_client [-h IPv4-address] [-p port] [-g game]";
    constexpr std::string_view game = "rtype";

    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        std::cout << usage << std::endl;
        return 0;
    }
    try {
        const engine::CommandLine options(argc, argv, usage, true, game);
        const std::filesystem::path data = engine::AssetPath::findData(argv[0]);
        const auto settings = engine::network::NetworkSettings::fromGame(
            engine::scene::DataNode::fromFile(data / (options.game() + "_game.json")));
        const engine::net::Endpoint server = engine::net::UdpTransport::numericEndpoint(
            options.host(), options.port().value_or(settings.defaultPort()));
        engine::client::OnlineClient client(engine::AssetPath::findAssets(argv[0]), data, options.game(), server, settings);
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
