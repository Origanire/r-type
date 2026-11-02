#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "GameView.hpp"
#include "NetworkClient.hpp"
#include "RtypeProtocol.hpp"

namespace rtype::client {

class Client
{
public:
    Client(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
        const std::string &game, engine::net::Endpoint server);

    void run();

private:
    std::uint8_t buttons() const;
    void handleEvent(const protocol::Packet &event);
    void applySnapshot(const protocol::WorldSnapshot &snapshot);

    engine::client::GameView m_view;
    NetworkClient m_network;
};

} // namespace rtype::client
