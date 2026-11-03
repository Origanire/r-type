#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>

#include "AssetPath.hpp"
#include "CommandLine.hpp"
#include "DataNode.hpp"
#include "EngineError.hpp"
#include "GameServer.hpp"
#include "NetworkSettings.hpp"
#include "ServerRunner.hpp"
#include "Simulation.hpp"

int main(int argc, char **argv)
{
    constexpr std::string_view usage = "Usage: r-type_server [-p port] [-g game]";
    constexpr std::string_view game = "rtype";

    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        std::cout << usage << std::endl;
        return 0;
    }
    try {
        const engine::CommandLine options(argc, argv, usage, false, game);
        const std::filesystem::path data = engine::AssetPath::findData(argv[0]);
        const auto settings = engine::network::NetworkSettings::fromGame(
            engine::scene::DataNode::fromFile(data / (options.game() + "_game.json")));
        engine::simulation::Simulation simulation(data, options.game(), settings);
        engine::network::GameServer server(options.port().value_or(settings.defaultPort()), settings, simulation);
        engine::network::ServerRunner::run(server, settings.tickRate());
    } catch (const engine::EngineError &error) {
        std::cerr << "r-type_server: " << error.what() << std::endl;
        return 84;
    } catch (const std::exception &error) {
        std::cerr << "r-type_server: unexpected error: " << error.what() << std::endl;
        return 84;
    }
    return 0;
}
