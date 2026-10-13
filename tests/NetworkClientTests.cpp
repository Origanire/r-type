#include <chrono>
#include <iostream>
#include <thread>

#include "NetworkClient.hpp"
#include "PortOption.hpp"

int main(int argc, char **argv)
{
    try {
        if (argc != 2)
            return 1;
        rtype::client::NetworkClient client({"127.0.0.1", rtype::parsePort(argv[1])});
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        bool moved = false;
        while (std::chrono::steady_clock::now() < deadline && !moved) {
            client.update(8);
            if (auto snapshot = client.takeSnapshot())
                for (const auto &entity : snapshot->entities)
                    moved = moved || entity.x >= 240;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        client.disconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (!moved)
            throw std::runtime_error("NetworkClient did not receive authoritative movement");
        std::cout << "NetworkClient test passed\n";
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
