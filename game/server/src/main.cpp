#include <chrono>
#include <csignal>
#include <exception>
#include <iostream>
#include <string_view>
#include <stdexcept>
#include <thread>

#include "PortOption.hpp"
#include "Server.hpp"

namespace {
volatile std::sig_atomic_t running = 1;
void stop(int) { running = 0; }
}

int main(int argc, char **argv)
{
    try {
        std::uint16_t port = rtype::protocol::DEFAULT_PORT;
        for (int i = 1; i < argc; ++i) {
            if (std::string_view(argv[i]) == "--help") {
                std::cout << "Usage: r-type_server [-p port]\n";
                return 0;
            }
            if (std::string_view(argv[i]) != "-p" || ++i >= argc)
                throw std::runtime_error("Usage: r-type_server [-p port]");
            port = rtype::parsePort(argv[i]);
        }
        std::signal(SIGINT, stop); std::signal(SIGTERM, stop);
        rtype::server::Server server(port);
        using Clock = std::chrono::steady_clock;
        constexpr auto tick = std::chrono::nanoseconds(16666667);
        auto next = Clock::now();
        while (running) {
            const auto now = Clock::now();
            server.step(now);
            next += tick;
            // Bound catch-up after suspension; network reception never blocks simulation.
            if (now - next > std::chrono::milliseconds(250))
                next = now + tick;
            std::this_thread::sleep_until(next);
        }
        server.shutdown();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    } catch (const std::exception &error) {
        std::cerr << "r-type_server: " << error.what() << '\n';
        return 84;
    }
    return 0;
}
