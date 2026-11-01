#include <chrono>
#include <csignal>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include "AssetPath.hpp"
#include "Clock.hpp"
#include "EngineError.hpp"
#include "FixedTimestep.hpp"
#include "GameError.hpp"
#include "PortOption.hpp"
#include "RtypeProtocol.hpp"
#include "Server.hpp"

namespace {

constexpr std::string_view USAGE = "Usage: r-type_server [-p port]";
constexpr std::chrono::milliseconds IDLE_DELAY{1};

volatile std::sig_atomic_t running = 1;

void stop(int)
{
    running = 0;
}

std::uint16_t parseArguments(int argc, char **argv)
{
    std::uint16_t port = rtype::protocol::DEFAULT_PORT;
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) != "-p" || i + 1 >= argc)
            throw rtype::GameError(std::string(USAGE));
        port = rtype::parsePort(argv[++i]);
    }
    return port;
}

} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        std::cout << USAGE << std::endl;
        return 0;
    }
    try {
        rtype::server::Server server(parseArguments(argc, argv), rtype::findData(argv[0]));
        std::signal(SIGINT, stop);
        std::signal(SIGTERM, stop);
        engine::Clock clock;
        engine::FixedTimestep timestep(1.f / static_cast<float>(rtype::protocol::TICK_RATE));
        while (running) {
            server.poll();
            timestep.addTime(clock.restart());
            while (timestep.step())
                server.tick(timestep.stepSeconds());
            std::this_thread::sleep_for(IDLE_DELAY);
        }
        server.shutdown();
    } catch (const engine::EngineError &error) {
        std::cerr << "r-type_server: " << error.what() << std::endl;
        return 84;
    } catch (const std::exception &error) {
        std::cerr << "r-type_server: unexpected error: " << error.what() << std::endl;
        return 84;
    }
    return 0;
}
